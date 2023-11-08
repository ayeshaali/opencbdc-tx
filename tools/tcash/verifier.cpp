#include "verifier.hpp"
#include <string>
#include <gmp.h>

struct Proof {
    libff::alt_bn128_G1 A;
    libff::alt_bn128_G2 B;
    libff::alt_bn128_G1 C;
};

namespace cbdc::parsec {
    verifier::verifier(std::shared_ptr<logging::log> log) 
              : m_log(std::move(log)) {
        m_log->trace("verifier");
        libff::alt_bn128_pp::init_public_params();
    }

    auto verifier::verifyProof(std::string proof, 
                               std::string _root, 
                               std::string _nullifierHash, 
                               std::string _recipient, 
                               std::string _relayer = "0", 
                               int _fee = 0,
                               int _refund = 0) -> bool {

        for (size_t i = 0; i < 8; i++) {
            mpz_t _mpz_p;
            mpz_init(_mpz_p);
            mpz_set_str(_mpz_p, proof.substr(i*64, 64).c_str(), 16);
            p[i] = libff::alt_bn128_Fq(bigint_q(_mpz_p));
        }

        std::array<bigint_r, 6> inputs;
        constructInputList(inputs, _root, _nullifierHash, _recipient, _relayer, _fee, _refund);

        struct Proof _proof = {
            libff::alt_bn128_G1(p[0], p[1], libff::alt_bn128_Fq::one()),
            libff::alt_bn128_G2(libff::alt_bn128_Fq2(p[2],p[3]), libff::alt_bn128_Fq2(p[4],p[5]), libff::alt_bn128_Fq2::one()),
            libff::alt_bn128_G1(p[6], p[7], libff::alt_bn128_Fq::one())
        };

        libff::alt_bn128_G1 vk_x = libff::alt_bn128_G1(libff::alt_bn128_Fq(bigint_q("0")), libff::alt_bn128_Fq(bigint_q("0")), libff::alt_bn128_Fq::one());
        vk_x = vk_x + vk.IC[0];

        for (size_t i = 0; i < 6; i++) {
            vk_x = vk_x + (inputs[i] * vk.IC[i + 1]);// Pairing.scalar_mul(vk.IC[i + 1], input[i]));
        }

        libff::alt_bn128_Fq12 x = libff::alt_bn128_Fq12::one();
        std::array<std::pair<libff::alt_bn128_G1, libff::alt_bn128_G2>, 4> pairs = {
            std::make_pair(-_proof.A, _proof.B), 
            std::make_pair(vk.alfa1, vk.beta2), 
            std::make_pair(vk_x, vk.gamma2), 
            std::make_pair(_proof.C, vk.delta2)
        }; 

        for (size_t i=0; i < 4; i++) {
            libff::alt_bn128_G1 g1 = pairs[i].first;
            libff::alt_bn128_G2 p = pairs[i].second;
            if (-libff::alt_bn128_G2::scalar_field::one() * p + p != libff::alt_bn128_G2::zero())
                return false;
            if (p.is_zero() || g1.is_zero())
                continue; // the pairing is one
            x = x * libff::alt_bn128_miller_loop(
                libff::alt_bn128_precompute_G1(g1),
                libff::alt_bn128_precompute_G2(p)
            );
        }
        
        return libff::alt_bn128_final_exponentiation(x) == libff::alt_bn128_GT::one();
    }

    void verifier::constructInputList(std::array<bigint_r, 6> inputs, 
                            std::string _root, 
                            std::string _nullifierHash, 
                            std::string _recipient, 
                            std::string _relayer, 
                            int _fee,
                            int _refund) {
        mpz_t _mpz_root;
        mpz_init(_mpz_root);
        mpz_set_str(_mpz_root, _root.c_str(), 16);
        
        mpz_t _mpz_nullifier;
        mpz_init(_mpz_nullifier);
        mpz_set_str(_mpz_nullifier, _nullifierHash.c_str(), 16);

        mpz_t _mpz_recipient;
        mpz_init(_mpz_recipient);
        mpz_set_str(_mpz_recipient, _recipient.c_str(), 16);

        mpz_t _mpz_relayer;
        mpz_init(_mpz_relayer);
        mpz_set_str(_mpz_relayer, _relayer.c_str(), 16);

        // convert left and right into bigInt
        bigint_r int_root = bigint_r(_mpz_root);
        bigint_r int_nullifier = bigint_r(_mpz_nullifier);      
        bigint_r int_recipient = bigint_r(_mpz_recipient);      
        bigint_r int_relayer = bigint_r(_mpz_relayer);          
        bigint_r int_fee = bigint_r(_fee);          
        bigint_r int_refund = bigint_r(_refund);   

        inputs = {int_root, int_nullifier, int_recipient, int_relayer, int_fee, int_refund};
    }
}
