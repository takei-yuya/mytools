#include <iostream>
#include <fstream>

#include <vector>
#include <string>

#include <getopt.h>

const std::string kDefaultDelimiter = "\\t";

std::string UnescapeControls(const std::string& str) {
  std::string result;
  result.reserve(str.size());
  for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
    if (*it != '\\') {
      result += *it;
      continue;
    }
    ++it;
    if (it == str.end()) {
      break;
    }
    switch (*it) {
    case 'a': result += "\a"; break;
    case 'b': result += "\b"; break;
    case 'f': result += "\f"; break;
    case 'n': result += "\n"; break;
    case 'r': result += "\r"; break;
    case 't': result += "\t"; break;
    case 'v': result += "\v"; break;
    case '\\': result += "\\"; break;
    default: result += *it; break;
    }
  }
  return result;
}

struct Options {
  std::string delimiter;
};

void Usage(std::ostream& out, int argc, char** argv) {
  out
    << "Usage: " << argv[0] << " [OPTIONS] [FILES]..." << std::endl
    << "Like paste(1), join FILES horizontally with delimiter, but stop at the end of shortest file." << std::endl
    << std::endl
    << "  -d, --delimiter=DELIM Paster lines with DELIM [default: '" << kDefaultDelimiter << "']" << std::endl
    ;
}

bool GetLineSet(std::vector<std::istream*>& ins, std::vector<std::string>& lines) {
  std::vector<std::string> ls;
  if (ins.empty()) {
    return false;
  }

  for (std::vector<std::istream*>::iterator it = ins.begin(); it != ins.end(); ++it) {
    std::string line;
    if (!std::getline(**it, line)) {
      return false;
    }
    ls.push_back(line);
  }
  lines.swap(ls);
  return true;
}

void Paster(const Options& options, std::vector<std::istream*>& ins, std::ostream& out) {
  std::vector<std::string> lines;
  while(GetLineSet(ins, lines)) {
    for (std::vector<std::string>::const_iterator it = lines.begin(); it != lines.end(); ++it) {
      if (it != lines.begin()) {
        out << options.delimiter;
      }
      out << *it;
    }
    out << std::endl;
  }
}

int main(int argc, char** argv) {
  Options options;
  options.delimiter = "\t";

  while (true) {
    static struct option log_options[] = {
      { "delimiter", required_argument, 0, 'd' },
      { "help", no_argument, 0, '!' },
      { 0, 0, 0, 0 },
    };

    int c = getopt_long(argc, argv, "d:", log_options, NULL);
    if (c == -1) break;

    switch(c) {
    case 'd':
      options.delimiter = optarg;
      break;

    case '!':
      Usage(std::cout, argc, argv);
      return 0;

    default:
      Usage(std::cerr, argc, argv);
      return 0;
    }
  }

  std::vector<std::istream*> ins;
  for (size_t i = optind; i < argc; ++i) {
    ins.push_back(new std::ifstream(argv[i]));
    if (!*ins.back()) {
      std::cerr << "Failed to open \"" << argv[i] << "\"." << std::endl;
      Usage(std::cerr, argc, argv);
      return 1;
    }
  }

  if (ins.empty()) {
    std::cerr << "No file given" << std::endl;
    Usage(std::cerr, argc, argv);
    return 1;
  }

  options.delimiter = UnescapeControls(options.delimiter);
  Paster(options, ins, std::cout);

  while (!ins.empty()) {
    delete ins.back();
    ins.pop_back();
  }
}
