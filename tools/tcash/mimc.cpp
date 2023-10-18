#include "mimc.hpp"

#include <string>
#include <gmp.h>

namespace cbdc::parsec {
    mimc::mimc(std::shared_ptr<logging::log> log) 
              : m_log(std::move(log)) {
        m_log->trace("hasher");
    }

    auto mimc::hash(std::string left, std::string right) -> std::string {
        // convert left and right from hex
        mpz_t _mpz_left;
        mpz_set_str(_mpz_left, left.c_str(), 16);
        mpz_t _mpz_right;
        mpz_set_str(_mpz_right, right.c_str(), 16);
        // convert left and right into bigInt
        uint256 _left = uint256(_mpz_left);
        uint256 _right = uint256(_mpz_right);

        (void)_right;
        // uint256 R, C;
        // std::tie(R,C) = MiMC5Sponge(_left, uint256("0"), uint256("0"));
        // std::tie(R,C) = MiMC5Sponge(_right, C, uint256("0"));
        mpz_t output;
        _left.to_mpz(output);
        char* hash = mpz_get_str(NULL, 16, output);;
        return hash;
    }

    auto mimc::MiMC5Sponge(uint256 _L, uint256 _R, uint256 _k) -> std::pair<uint256, uint256> {
        // hash function according to circomlib mimcsponge
        Fr xR = Fr(_R);
        Fr xL = Fr(_L);
        Fr k  = Fr(_k);

        for (size_t i=0; i<nRounds; i++) {
            Fr cst = Fr(c[i]);
            Fr t; 
            if (i == 0) {
                t = xL+k;
            } else {
                t = xL+k+cst;
            }
            Fr xR_tmp = Fr(xR);
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
