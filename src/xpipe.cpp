#include <getopt.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

struct Options {
  Options()
    : line(1)
    , parallel(false) {
  }

  int line;
  bool parallel;
};

void Usage(std::ostream& out, int argc, char** argv) {
  Options options;  // default options
  out
    << "Usage: " << argv[0] << " [OPTIONS]... COMMAND [ARGS]..." << std::endl
    << "Run COMMAND and send data from input" << std::endl
    << std::endl
    << "  -l, --line=LINE       Run COMMAND per input LINE lines.[default: " << options.line << "]" << std::endl
    << "  -p, --parallel        Run COMMAND parallel." << std::endl;
}

pid_t Execute(char *argv[], int& wfd) {
  int fds[2];  // r w
  pipe(fds);
  pid_t pid = fork();

  if (pid == -1) {
    perror("fork()");
    return -1;
  }

  if (pid != 0) {  // parent
    close(fds[0]);
    wfd = fds[1];
    return pid;
  }

  // TODO(takei): expand argument with some id
  //   e.g. "xpipe -- command -o %d.out" will run "command -o 1.out" "command -o 2.out" ... etc

  // child
  close(fds[1]);
  dup2(fds[0], STDIN_FILENO);
  execvp(argv[0], argv);
  perror("exec()");
  exit(EXIT_FAILURE);
}

bool Read(const Options& options, std::istream& in, std::string& result) {
  bool read_some = false;
  std::string buf;

  // TODO(takei): more read type (e.g. bytes, tokens, etc)
  std::string line;
  for (size_t i = 0; i < options.line && std::getline(in, line); ++i) {
    read_some = true;
    buf += line + "\n";
  }

  result.swap(buf);
  return read_some;
}

int main(int argc, char** argv) {
  Options options;

  while (true) {
    static struct option log_options[] = {
      { "line", required_argument, 0, 'l' },
      { "parallel", no_argument, 0, 'p'},
      { "help", no_argument, 0, '!' },
      { 0, 0, 0, 0 },
    };

    int c = getopt_long(argc, argv, "+l:w", log_options, NULL);
    if (c == -1) break;

    switch (c) {
    case 'l':
      {
        std::istringstream ss(optarg);
        ss >> options.line;
        if (options.line <= 0) {
          std::cerr << "Invalid value for --line=" << std::endl;
          exit(EXIT_FAILURE);
        }
      }
      break;

    case 'p':
      options.parallel = true;
      break;

    case '!':
      Usage(std::cout, argc, argv);
      exit(EXIT_SUCCESS);

    default:
      Usage(std::cerr, argc, argv);
      exit(EXIT_FAILURE);
    }
  }

  if (optind >= argc) {
    Usage(std::cerr, argc, argv);
    exit(EXIT_FAILURE);
  }

  std::vector<pid_t> pids;
  std::string buf;
  while (Read(options, std::cin, buf)) {
    int fd;
    pid_t pid = Execute(argv + optind, fd);
    if (pid < 0) {
      exit(EXIT_FAILURE);
    }
    pids.push_back(pid);
    write(fd, buf.c_str(), buf.size());
    close(fd);

    // TODO(takei): check execvp status even if parallel
    if (!options.parallel) {
      int status;
      waitpid(pid, &status, 0);
      if (status != 0) {
        std::cerr << "Subprocess " << pid << " exit with error code: " << status << std::endl;
        exit(EXIT_FAILURE);
      }
    }
  }

  for (std::vector<pid_t>::const_iterator it = pids.begin(); it != pids.end(); ++it) {
    int status;
    waitpid(*it, &status, 0);
    if (status != 0) {
      std::cerr << "Subprocess " << *it << " exit with error code: " << status << std::endl;
    }
  }
}
