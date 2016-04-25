#include <iostream>
#include <string>
#include <vector>
#include <ctime>

#include <getopt.h>


// NOTE:
//   Since there are no way to detect strftime is failed by short buffer or generate empty string successfully,
//   add dummy suffix not to generate empty string.
const std::string kDummySuffix_strftime = " ";

const std::string kDefaultPrefix = "%F %T%t";
const std::string kDefaultSuffix = "";

struct Options {
  std::string prefix;
  std::string suffix;
};

void Usage(std::ostream& out, int argc, char** argv) {
  out
    << "Usage: " << argv[0] << " [OPTIONS]" << std::endl
    << std::endl
    << "  -p, --prefix=PREFIX   Insert PREFIX before each line. [default: '" << kDefaultPrefix << "']" << std::endl
    << "  -s, --suffix=SUFFIX   Append SUFFIX after each line. [default: '" << kDefaultSuffix << "']" << std::endl
    ;
}

std::string FormatTime(const std::string& str, const struct tm& tm) {
  std::vector<char> buf(128);
  size_t sz;
  while ((sz = strftime(buf.data(), buf.size(), (str + kDummySuffix_strftime).c_str(), &tm)) == 0) {
    buf.resize(buf.size() * 2 + 1);
  }
  return std::string(buf.data(), sz - kDummySuffix_strftime.size());
}

void Timestamper(const Options& options, std::istream& in, std::ostream& out) {
  std::string line;
  while (std::getline(in, line)) {
    time_t t = time(NULL);
    struct tm tm;
    gmtime_r(&t, &tm);

    out
      << FormatTime(options.prefix, tm)
      << line
      << FormatTime(options.suffix, tm)
      << std::endl;
  }
}

int main(int argc, char **argv) {
  Options options;
  options.prefix = kDefaultPrefix;
  options.suffix = kDefaultSuffix;

  while (true) {
    static struct option log_options[] = {
      { "prefix", required_argument, 0, 'p' },
      { "suffix", required_argument, 0, 's' },
      { "help", no_argument, 0, '!' },
      { 0, 0, 0, 0 },
    };

    int c = getopt_long(argc, argv, "p:s:", log_options, NULL);
    if (c == -1) break;

    switch(c) {
    case 'p':
      options.prefix = optarg;
      break;

    case 's':
      options.suffix = optarg;
      break;

    case '!':
      Usage(std::cout, argc, argv);
      return 0;

    default:
      Usage(std::cerr, argc, argv);
      return 0;
    }
  }

  Timestamper(options, std::cin, std::cout);
  return 0;
}
