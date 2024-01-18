#include "util/common/logging.hpp"
#include <libff/algebra/fields/bigint.hpp>
#include <libff/algebra/curves/alt_bn128/alt_bn128_pp.hpp>
#include <iostream> 

typedef libff::bigint<libff::alt_bn128_r_limbs> bigint_r;
typedef libff::bigint<libff::alt_bn128_q_limbs> bigint_q;

struct VerifyingKey {
    libff::alt_bn128_G1 alfa1;
    libff::alt_bn128_G2 beta2;
    libff::alt_bn128_G2 gamma2;
    libff::alt_bn128_G2 delta2;
    libff::alt_bn128_G1 IC[7];
    VerifyingKey(): 
        alfa1(libff::alt_bn128_G1(libff::alt_bn128_Fq(bigint_q("20692898189092739278193869274495556617788530808486270118371701516666252877969")), libff::alt_bn128_Fq(bigint_q("11713062878292653967971378194351968039596396853904572879488166084231740557279")), libff::alt_bn128_Fq::one())),
        beta2(libff::alt_bn128_G2(libff::alt_bn128_Fq2(libff::alt_bn128_Fq(bigint_q("281120578337195720357474965979947690431622127986816839208576358024608803542")),libff::alt_bn128_Fq(bigint_q("12168528810181263706895252315640534818222943348193302139358377162645029937006"))), 
                                            libff::alt_bn128_Fq2(libff::alt_bn128_Fq(bigint_q("9011703453772030375124466642203641636825223906145908770308724549646909480510")),libff::alt_bn128_Fq(bigint_q("16129176515713072042442734839012966563817890688785805090011011570989315559913"))), 
                                            libff::alt_bn128_Fq2::one())),
        gamma2(libff::alt_bn128_G2(libff::alt_bn128_Fq2(libff::alt_bn128_Fq(bigint_q("10857046999023057135944570762232829481370756359578518086990519993285655852781")),libff::alt_bn128_Fq(bigint_q("11559732032986387107991004021392285783925812861821192530917403151452391805634"))), 
                                            libff::alt_bn128_Fq2(libff::alt_bn128_Fq(bigint_q("8495653923123431417604973247489272438418190587263600148770280649306958101930")),libff::alt_bn128_Fq(bigint_q("4082367875863433681332203403145435568316851327593401208105741076214120093531"))), 
                                            libff::alt_bn128_Fq2::one())),
        delta2(libff::alt_bn128_G2(libff::alt_bn128_Fq2(libff::alt_bn128_Fq(bigint_q("150879136433974552800030963899771162647715069685890547489132178314736470662")),libff::alt_bn128_Fq(bigint_q("21280594949518992153305586783242820682644996932183186320680800072133486887432"))), 
                                            libff::alt_bn128_Fq2(libff::alt_bn128_Fq(bigint_q("11434086686358152335540554643130007307617078324975981257823476472104616196090")),libff::alt_bn128_Fq(bigint_q("1081836006956609894549771334721413187913047383331561601606260283167615953295"))), 
                                            libff::alt_bn128_Fq2::one())),
        IC{
            libff::alt_bn128_G1(libff::alt_bn128_Fq(bigint_q("16225148364316337376768119297456868908427925829817748684139175309620217098814")), libff::alt_bn128_Fq(bigint_q("5167268689450204162046084442581051565997733233062478317813755636162413164690")), libff::alt_bn128_Fq::one()),
            libff::alt_bn128_G1(libff::alt_bn128_Fq(bigint_q("12882377842072682264979317445365303375159828272423495088911985689463022094260")), libff::alt_bn128_Fq(bigint_q("19488215856665173565526758360510125932214252767275816329232454875804474844786")), libff::alt_bn128_Fq::one()),
            libff::alt_bn128_G1(libff::alt_bn128_Fq(bigint_q("13083492661683431044045992285476184182144099829507350352128615182516530014777")), libff::alt_bn128_Fq(bigint_q("602051281796153692392523702676782023472744522032670801091617246498551238913")), libff::alt_bn128_Fq::one()),
            libff::alt_bn128_G1(libff::alt_bn128_Fq(bigint_q("9732465972180335629969421513785602934706096902316483580882842789662669212890")), libff::alt_bn128_Fq(bigint_q("2776526698606888434074200384264824461688198384989521091253289776235602495678")), libff::alt_bn128_Fq::one()),
            libff::alt_bn128_G1(libff::alt_bn128_Fq(bigint_q("8586364274534577154894611080234048648883781955345622578531233113180532234842")), libff::alt_bn128_Fq(bigint_q("21276134929883121123323359450658320820075698490666870487450985603988214349407")), libff::alt_bn128_Fq::one()),
            libff::alt_bn128_G1(libff::alt_bn128_Fq(bigint_q("4910628533171597675018724709631788948355422829499855033965018665300386637884")), libff::alt_bn128_Fq(bigint_q("20532468890024084510431799098097081600480376127870299142189696620752500664302")), libff::alt_bn128_Fq::one()),
            libff::alt_bn128_G1(libff::alt_bn128_Fq(bigint_q("15335858102289947642505450692012116222827233918185150176888641903531542034017")), libff::alt_bn128_Fq(bigint_q("5311597067667671581646709998171703828965875677637292315055030353779531404812")), libff::alt_bn128_Fq::one())
        }
        {}
};

namespace cbdc {
    class verifier {
        public:
            verifier(std::shared_ptr<logging::log> log);
            auto verifyProof(std::string proof, 
                             std::string _root, 
                             std::string _nullifierHash, 
                             std::string _recipient, 
                             std::string _relayer, 
                             int _fee,
                             int _refund) -> bool;
                             
        
        private:
            std::shared_ptr<logging::log> m_log;
            libff::alt_bn128_Fq p[8]; 
            struct VerifyingKey vk;

            void constructInputList(bigint_r (&inputs)[6], 
                                    std::string _root, 
                                    std::string _nullifierHash, 
                                    std::string _recipient, 
                                    std::string _relayer, 
                                    int _fee,
                                    int _refund);
    };
}
