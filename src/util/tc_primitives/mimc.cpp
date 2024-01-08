#include "mimc.hpp"
#include <string>
#include <gmp.h>

namespace cbdc {
    mimc::mimc(std::shared_ptr<logging::log> log) 
              : m_log(std::move(log)) {
        libff::alt_bn128_pp::init_public_params();
    }

    auto mimc::hash(std::string left, std::string right) -> std::string {
        // convert left and right from hex
        mpz_t _mpz_left;
        mpz_init(_mpz_left);
        mpz_set_str(_mpz_left, left.c_str(), 16);
        mpz_t _mpz_right;
        mpz_init(_mpz_right);
        mpz_set_str(_mpz_right, right.c_str(), 16);
        // convert left and right into bigInt
        bigint_r _left = bigint_r(_mpz_left);
        bigint_r _right = bigint_r(_mpz_right);

        // do hashing according to solidity contract
        bigint_r R, C;
        std::tie(R,C) = MiMC5Sponge(_left, bigint_r("0"), bigint_r("0"));

        libff::alt_bn128_Fr fR = libff::alt_bn128_Fr(R);
        libff::alt_bn128_Fr fRight = libff::alt_bn128_Fr(_right);

        bigint_r R_sum = (fR+fRight).as_bigint();
        std::tie(R,C) = MiMC5Sponge(R_sum, C, bigint_r("0"));

        // convert back to hex
        mpz_t output;
        mpz_init(output);
        R.to_mpz(output);
        char* hash = mpz_get_str(NULL, 16, output);
        return hash;
    }

    auto mimc::MiMC5Sponge(bigint_r _L, bigint_r _R, bigint_r _k) -> std::pair<bigint_r, bigint_r> {
        // hash function according to circomlib mimcsponge
        libff::alt_bn128_Fr xR = libff::alt_bn128_Fr(_R);
        libff::alt_bn128_Fr xL = libff::alt_bn128_Fr(_L);
        libff::alt_bn128_Fr k  = libff::alt_bn128_Fr(_k);

        for (size_t i=0; i<nRounds; i++) {
            libff::alt_bn128_Fr cst = libff::alt_bn128_Fr(c[i]);
            libff::alt_bn128_Fr t; 
            if (i == 0) {
                t = xL+k;
            } else {
                t = xL+k+cst;
            }
            libff::alt_bn128_Fr xR_tmp = libff::alt_bn128_Fr(xR);
            if (i < (nRounds - 1)) {
                xR = xL;
                xL = xR_tmp + (t^5);
            } else {
                xR = xR_tmp + (t^5);
            }
        }

        return std::make_pair(xL.as_bigint(),xR.as_bigint());
    }
}
