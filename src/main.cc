/**
 * @file main.cc
 * @license MIT
 * @std >=c++17
 * @author stfwi
 *
 * GUID Generator CLI application.
 */
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <string>
#include <random>
#include <array>
#include <cstdint>

#ifndef PROGRAM_NAME
  #define PROGRAM_NAME "guid-generate"
#endif

#ifndef PROGRAM_VERSION
  #define PROGRAM_VERSION "1.0.0"
#endif

#ifndef SEED_OFFSET
  #define SEED_OFFSET (0x271d8a39u)
#endif


namespace arith { namespace {

  /**
   * "Forward implementation" of std::rotl, which will be
   * available in c++20.
   * @tparam typename T | concept Integral
   * @param T x
   * @param int s
   * @return T
   */
  template<typename T>
  constexpr T rotl(const T x, const int s) noexcept
  {
    static_assert(std::is_integral_v<T>, "Value to rotate must be an integer.");
    constexpr auto N = int(sizeof(T) * 8);
    const int r = s % N;
    return (r==0) ? (x) : ((r>0) ? ( (x<<r)|(x>>((N-r)%N)) ) : ( (x>>(-r))|(x<<((N+r)%N)) ));
  }
}}

/**
 * Basic data type used for the binary UID sequence.
 */
typedef std::array<uint8_t, 16> guid_data_t;

/**
 * GUID generator, if the `seed` is empty, a random
 * seed will be used.
 * @tparam typename SeedContainer
 * @param SeedContainer&& seed
 * @return guid_data_t
 */
namespace {
  template<typename SeedContainer>
  guid_data_t generate_guid(SeedContainer&& seed)
  {
    using namespace std;
    using seed_type = typename std::decay_t<decltype(mt19937::default_seed)>;
    static_assert(sizeof(seed_type) >= sizeof(uint32_t), "Seeds need to be at least 32bit.");
    // Seed generation based on accumulation initialization offset and fixed iterative bit rotation,
    // which shall allow to spread the string/container `seed` input over the whole mersenne seed
    // range for different rnd instances.
    const auto mkseed = [](const auto& seed, const seed_type ofs, const int rot){
      if(seed.empty()) {
        return arith::rotl(seed_type(random_device()()), rot); // Rotate also here in case rnd is time based.
      } else {
        return accumulate(seed.begin(), seed.end(), seed_type(ofs), [rot](const auto acc, const auto b){
          return arith::rotl(acc ^ ((acc<<8u)|seed_type(b)), rot);
        });
      }
    };
    // Permutated 4x 32bit seeded mersenne twisters to cover 128bit GUID from 32bit random engines.
    // According to spec `mt19937` is deterministic/shall be identical for all implementations/platforms.
    auto mt = array<mt19937, 4>();
    mt[0] = mt19937(mkseed(seed, (SEED_OFFSET), 0));
    mt[1] = mt19937(mkseed(seed, mt[0](), 7));
    mt[2] = mt19937(mkseed(seed, mt[1](), 11));
    mt[3] = mt19937(mkseed(seed, mt[2](), 13));
    // Data generation eith cyclic rnd selection.
    auto data = guid_data_t();
    auto i = 0;
    for(auto& e:data) e = typename guid_data_t::value_type(mt[(++i) & 0x3]());
    return data;
  }
}

/**
 * Returns the string representation of binary GUID data.
 * Format is "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX".
 * @param const guid_data_t& data
 * @return std::string
 */
namespace {
  std::string to_string(const guid_data_t& data)
  {
    using namespace std;
    auto ss = stringstream(); // c++20 format not yet avail.
    auto i = size_t(0);
    for(const auto e:data) {
      ss << setfill('0') << setw(2) << uppercase << hex << int(e);
      switch(++i) {
        case 4: case 6: case 8: case 10: { ss << "-"; break; }
        default: break;
      }
    }
    return ss.str();
  }
}

/**
 * Returns the concatenated application arguments, space separated.
 * @tparam typename StringType
 * @param int argc
 * @param char** argv
 * @return StringType
 */
namespace {
  template <typename StringType>
  StringType arguments(int argc, char** argv)
  {
    if((argc<1) || !argv[1]) return StringType();
    auto args = StringType(argv[1]);
    for(auto i=int(2); i<argc && argv[i]; ++i) { args += " "; args += argv[i]; }
    return args;
  }
}

/**
 * Application main entry point.
 * @param int argc
 * @param char** argv
 * @return int
 */
int main(int argc, char** argv)
{
  using namespace std;
  auto arg = arguments<string>(argc, argv);
  auto num_outputs = 1;
  if(arg.empty()) {
    ; // No further argument checks needed.
  } else if(arg.find("-n") == 0) {
    // Simple parsing from 1st number, covers -n 10, -n10, -n=10, further arguments are ignored.
    const auto pos = arg.find_first_of("0123456789");
    if(pos == arg.npos) {
      cerr << "Invalid option -n, missing number of output lines.\n";
      return 1;
    } else {
      num_outputs = std::atol(&arg.c_str()[pos]);
      arg.clear(); // Always random, implies empty seed.
    }
  } else if(arg=="--version" || arg=="-v") {
    cout << (PROGRAM_NAME) << " version " << (PROGRAM_VERSION) << ".\n";
    return 0;
  } else if(arg=="--help" || arg=="-h" || arg=="/?") {
    cout << "Usage: " << (PROGRAM_NAME) << " [-h|--help|-n <lines>|<seed-string>]\n\n"
         << "  <seed-string>: (1st arg no dash `-`): Text bytes used as seed.\n"
         << "  -n <lines>   : Generate `lines` random output lines.\n"
         << "  -h, --help   : Show this help.\n"
         << "\n"
         << "The program generates random or text seeded GUIDs, where the output\n"
         << "format is \"XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX\". For argument defined\n"
         << "seed strings, it is recommended to use at least 16 characters.\n"
         << "The integrated seed initialization value compiled with this this\n"
         << "binary is 0x" << hex << (SEED_OFFSET) << ". (Binaries with different seed init\n"
         << "will produce different outputs for the same given seed text).\n"
         << "\n";
    return 0;
  } else if(arg.front() == '-') {
    cerr << "Invalid option '" << arg << "', try --help\n";
    return 1;
  }
  while(--num_outputs >= 0) {
    cout << to_string(generate_guid(arg)) << "\n";
  }
  return 0;
}
