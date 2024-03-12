// Copyright (c) 2021 MIT Digital Currency Initiative,
//                    Federal Reserve Bank of Boston
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "util/common/logging.hpp"
#include <libff/algebra/fields/bigint.hpp>
#include <libff/algebra/curves/alt_bn128/alt_bn128_pp.hpp>
#include <string>

typedef libff::bigint<libff::alt_bn128_r_limbs> bigint_r;
typedef libff::bigint<libff::alt_bn128_q_limbs> bigint_q;

namespace cbdc {
    class blind_sig {
      public:
        /// Constructor.
        blind_sig(std::shared_ptr<logging::log> log);
        
        auto hash2curve(std::string sn) -> libff::alt_bn128_G1; 

        auto sign(std::string str_x, std::string str_y) -> std::string;

        auto verify(std::string sn, std::string str_sig) -> bool;

        auto blind(libff::alt_bn128_G1 hash, std::string secret) -> libff::alt_bn128_G1;

        auto bigIntq2str(bigint_q Fq) -> std::string;
        
      private:
        std::shared_ptr<logging::log> m_log;
        libff::alt_bn128_Fr sk;
        libff::alt_bn128_G2 pk;
        bigint_q exp;

        auto str2Fq(std::string str_x, int base) -> libff::alt_bn128_Fq;
        auto str2Fr(std::string str_x, int base) -> libff::alt_bn128_Fr;

    };
}
