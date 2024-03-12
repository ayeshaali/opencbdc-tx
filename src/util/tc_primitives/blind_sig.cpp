#include "blind_sig.hpp"
#include <string>
#include <gmpxx.h>
#include "crypto/sha256.h"
#include "util/common/hash.hpp"


namespace cbdc {
    blind_sig::blind_sig(std::shared_ptr<logging::log> log)
                             : m_log(std::move(log)) {
        
        libff::alt_bn128_pp::init_public_params();
        sk = libff::alt_bn128_Fr(bigint_r("210678092281307515714425708160684607266613743544556181346141138690930841682"));
        pk = sk * libff::alt_bn128_G2::one();
        exp = libff::alt_bn128_Fq(bigint_q("5472060717959818805561601436314318772174077789324455915672259473661306552146")).as_bigint();
    }

    auto blind_sig::sign(std::string str_x, std::string str_y) -> std::string {
        libff::alt_bn128_Fq x = str2Fq(str_x, 16);
        libff::alt_bn128_Fq y = str2Fq(str_y, 16);

        libff::alt_bn128_G1 g1 = libff::alt_bn128_G1(x, y, libff::alt_bn128_Fq::one());
        libff::alt_bn128_G1 sig = sk * g1;
        sig.to_affine_coordinates();
        std::string output_x = bigIntq2str(sig.X.as_bigint());
        std::string output_y = bigIntq2str(sig.Y.as_bigint());

        std::string output;
        output.reserve(128);
        output.append(output_x);
        output.append(output_y);
        return output;
    }

    auto blind_sig::blind(libff::alt_bn128_G1 hash, std::string secret) -> libff::alt_bn128_G1 {
        libff::alt_bn128_Fr blinding_exp = str2Fr(secret, 16);
        libff::alt_bn128_G1 sig = blinding_exp * hash;
        sig.to_affine_coordinates();
        std::string output_x = bigIntq2str(sig.X.as_bigint());
        std::string output_y = bigIntq2str(sig.Y.as_bigint());
        return sig;
    }

    auto blind_sig::verify(std::string sn, std::string str_sig) -> bool {
        libff::alt_bn128_Fq x = str2Fq(str_sig.substr(0,64), 16);
        libff::alt_bn128_Fq y = str2Fq(str_sig.substr(64,128), 16);
        libff::alt_bn128_G1 sig = libff::alt_bn128_G1(x, y, libff::alt_bn128_Fq::one());

        libff::alt_bn128_G1 hash = hash2curve(sn);

         libff::alt_bn128_Fq12 p1 = libff::alt_bn128_miller_loop(
            libff::alt_bn128_precompute_G1(hash),
            libff::alt_bn128_precompute_G2(pk)
        );

        libff::alt_bn128_Fq12 p2 = libff::alt_bn128_miller_loop(
            libff::alt_bn128_precompute_G1(sig),
            libff::alt_bn128_precompute_G2(-libff::alt_bn128_G2::one())
        );

        libff::alt_bn128_Fq12 result = libff::alt_bn128_Fq12::one() * p1 * p2;

        return libff::alt_bn128_final_exponentiation(result) == libff::alt_bn128_GT::one();
    }

    auto blind_sig::hash2curve(std::string sn) -> libff::alt_bn128_G1 {
         
        std::string sha256_hash = cbdc::to_string(cbdc::hash_data((std::byte*) sn.c_str(), sn.size())).substr(0,62);
        
        libff::alt_bn128_Fq x = str2Fq(sha256_hash, 16);
        libff::alt_bn128_Fq z = (x^3) + 3;
        libff::alt_bn128_Fq y = z^exp;

        libff::alt_bn128_G1 g1 = libff::alt_bn128_G1(x, y, libff::alt_bn128_Fq::one());

        std::string output_x = bigIntq2str(g1.X.as_bigint());
        std::string output_y = bigIntq2str(g1.Y.as_bigint());

        return g1;
    }

    auto blind_sig::str2Fr(std::string str_x, int base) -> libff::alt_bn128_Fr {
        mpz_t _mpz;
        mpz_init(_mpz);
        mpz_set_str(_mpz, str_x.c_str(), base);
        libff::alt_bn128_Fr x = libff::alt_bn128_Fr(bigint_r(_mpz));
        mpz_clear(_mpz);
        return x;
    }

    auto blind_sig::str2Fq(std::string str_x, int base) -> libff::alt_bn128_Fq {
        mpz_t _mpz;
        mpz_init(_mpz);
        mpz_set_str(_mpz, str_x.c_str(), base);
        libff::alt_bn128_Fq x = libff::alt_bn128_Fq(bigint_q(_mpz));
        mpz_clear(_mpz);
        return x;
    }

    auto blind_sig::bigIntq2str(bigint_q Fq) -> std::string {
        mpz_t _mpz;
        mpz_init(_mpz);
        Fq.to_mpz(_mpz);

        char output[66];
        mpz_get_str(output, 16, _mpz);
        mpz_clear(_mpz);

        size_t str_length = strlen(output);
        size_t padding = 64 - str_length;
        if (padding > 0) {
            memmove(output+padding, output, str_length);
            for (size_t i =0; i < padding; i++) {
                output[i] = '0';
            }
        }

        return output;
    }

}
