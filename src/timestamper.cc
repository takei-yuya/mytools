#include <iostream>
#include <string>
#include <vector>
#include <ctime>

#include <getopt.h>

// NOTE: #strftime_empty_output
//   Since there are no way to detect strftime is failed by short buffer or generate empty string successfully,
//   add dummy suffix not to generate empty string.
const std::string kDummySuffix_strftime = " ";

const std::string kDefaultPrefix = "%F %T%t";
const std::string kDefaultSuffix = "";
const bool kDefaultUseLocalTime = false;

struct Options {
  Options()
    : prefix(kDefaultPrefix)
    , suffix(kDefaultSuffix)
    , use_localtime(kDefaultUseLocalTime) {
  }
  std::string prefix;
  std::string suffix;
  bool use_localtime;
};

void Usage(std::ostream& out, int argc, char** argv) {
  Options options;  // default options
  out
    << "Usage: " << argv[0] << " [OPTIONS]" << std::endl
    << "Add prefix and suffix per eachline. prefix and suffix parsed by strftime." << std::endl
    << std::endl
    << "  -p, --prefix=PREFIX   Insert PREFIX before each line. [default: '" << options.prefix << "']" << std::endl
    << "  -s, --suffix=SUFFIX   Append SUFFIX after each line. [default: '" << options.suffix << "']" << std::endl
    << "  -l, --localtime       Use localtime to generate prefix/suffix instead of gmtime." << std::endl
    ;
}

std::string FormatTime(const std::string& str, const struct tm& tm) {
  const std::string format = str + kDummySuffix_strftime; // #strftime_empty_output

  std::vector<char> buf(128);
  size_t sz;
  while ((sz = strftime(buf.data(), buf.size(), format.c_str(), &tm)) == 0) {
    buf.resize(buf.size() * 2 + 1);
  }
  return std::string(buf.data(), sz - kDummySuffix_strftime.size());
}

void Timestamper(const Options& options, std::istream& in, std::ostream& out) {
  std::string line;
  while (std::getline(in, line)) {
    time_t t = time(NULL);
    struct tm tm;
    if (options.use_localtime) {
      localtime_r(&t, &tm);
    } else {
      gmtime_r(&t, &tm);
    }

    out
      << FormatTime(options.prefix, tm)
      << line
      << FormatTime(options.suffix, tm)
      << std::endl;
  }
}

int main(int argc, char **argv) {
  Options options;

  while (true) {
    static struct option log_options[] = {
      { "prefix", required_argument, 0, 'p' },
      { "suffix", required_argument, 0, 's' },
      { "localtime", no_argument, 0, 'l' },
      { "help", no_argument, 0, '!' },
      { 0, 0, 0, 0 },
    };

    int c = getopt_long(argc, argv, "p:s:l", log_options, NULL);
    if (c == -1) break;

    switch(c) {
    case 'p':
      options.prefix = optarg;
      break;

    case 's':
      options.suffix = optarg;
      break;

    case 'l':
      options.use_localtime = true;
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
