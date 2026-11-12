#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#define MAX_STEPS 200000LL
#define H0 1.0e-4
#define RTOL 1.0e-6
#define ATOL 1.0e-9
static const double __attribute__((aligned(16))) FM_TAB[256] = {
    0x1.ffccdc2356379p-1, 0x1.ff66f00c7c3d4p-1, 0x1.ff017d45748c0p-1, 0x1.fe9c82c68d90cp-1, 0x1.fe37ff8b543ddp-1, 0x1.fdd3f29286af5p-1, 0x1.fd705ade07125p-1, 0x1.fd0d3772cecf9p-1,
    0x1.fcaa8758e1f5bp-1, 0x1.fc48499b42e35p-1, 0x1.fbe67d47e62ecp-1, 0x1.fb85216fa6c9dp-1, 0x1.fb2435263a609p-1, 0x1.fac3b78225f1ep-1, 0x1.fa63a79cb29fdp-1, 0x1.fa040491e2b71p-1,
    0x1.f9a4cd8066ec3p-1, 0x1.f946018993cd1p-1, 0x1.f8e79fd157660p-1, 0x1.f889a77e2f187p-1, 0x1.f82c17b91da31p-1, 0x1.f7ceefada159cp-1, 0x1.f7722e89aa8c0p-1, 0x1.f715d37d92199p-1,
    0x1.f6b9ddbc1033ap-1, 0x1.f65e4c7a3349ep-1, 0x1.f6031eef57224p-1, 0x1.f5a854551c1abp-1, 0x1.f54debe75e93ep-1, 0x1.f4f3e4e42e840p-1, 0x1.f49a3e8bc7308p-1, 0x1.f440f820870ecp-1,
    0x1.f3e810e6e7c9cp-1, 0x1.f38f8825766cap-1, 0x1.f3375d24cbb15p-1, 0x1.f2df8f2f84726p-1, 0x1.f2881d923a3f0p-1, 0x1.f231079b7c10ep-1, 0x1.f1da4c9bc7232p-1, 0x1.f183ebe57fe98p-1,
    0x1.f12de4cceb271p-1, 0x1.f0d836a827241p-1, 0x1.f082e0cf25020p-1, 0x1.f02de29ba22d4p-1, 0x1.efd93b6921ec2p-1, 0x1.ef84ea94e70a2p-1, 0x1.ef30ef7ded9f0p-1, 0x1.eedd4984e4f1cp-1,
    0x1.ee89f80c29756p-1, 0x1.ee36fa77bee0ep-1, 0x1.ede4502d4a604p-1, 0x1.ed91f8940cdf2p-1, 0x1.ed3ff314dd6bap-1, 0x1.ecee3f1a23b1cp-1, 0x1.ec9cdc0fd28e3p-1, 0x1.ec4bc96362b8dp-1,
    0x1.ebfb0683cd852p-1, 0x1.ebaa92e187ba0p-1, 0x1.eb5a6dee7c7e5p-1, 0x1.eb0a971e085bep-1, 0x1.eabb0de4f4569p-1, 0x1.ea6bd1b97118fp-1, 0x1.ea1ce21312342p-1, 0x1.e9ce3e6ac9748p-1,
    0x1.e97fe63ae248bp-1, 0x1.e931d8fefd3cbp-1, 0x1.e8e416340b86fp-1, 0x1.e8969d584aa82p-1, 0x1.e8496deb401cap-1, 0x1.e7fc876db5200p-1, 0x1.e7afe961b2813p-1, 0x1.e763934a7c88ap-1,
    0x1.e71784ac8eedbp-1, 0x1.e6cbbd0d98ddep-1, 0x1.e6803bf47912dp-1, 0x1.e63500e939f90p-1, 0x1.e5ea0b750de50p-1, 0x1.e59f5b224b58cp-1, 0x1.e554ef7c69570p-1, 0x1.e50ac80ffbc5dp-1,
    0x1.e4c0e46aafdeep-1, 0x1.e477441b48aeap-1, 0x1.e42de6b19ba05p-1, 0x1.e3e4cbbe8d185p-1, 0x1.e39bf2d40d1b7p-1, 0x1.e3535b8514033p-1, 0x1.e30b05659f3f1p-1, 0x1.e2c2f00aae227p-1,
    0x1.e27b1b0a3ebeap-1, 0x1.e23385fb4ac94p-1, 0x1.e1ec3075c48e7p-1, 0x1.e1a51a1293eefp-1, 0x1.e15e426b93695p-1, 0x1.e117a91b8d2efp-1, 0x1.e0d14dbe3843cp-1, 0x1.e08b2ff035a90p-1,
    0x1.e0454f4f0d92bp-1, 0x1.dfffab792ca7bp-1, 0x1.dfba440de14bfp-1, 0x1.df7518ad58f51p-1, 0x1.df3028f89d88bp-1, 0x1.deeb749192c4dp-1, 0x1.dea6fb1af3b1bp-1, 0x1.de62bc38501d6p-1,
    0x1.de1eb78e0a200p-1, 0x1.dddaecc153aa3p-1, 0x1.dd975b782c1b6p-1, 0x1.dd5403595de1ep-1, 0x1.dd10e40c7c22fp-1, 0x1.dccdfd39e06bep-1, 0x1.dc8b4e8aa86b4p-1, 0x1.dc48d7a8b3b27p-1,
    0x1.dc06983ea17f5p-1, 0x1.dbc48ff7ce8dfp-1, 0x1.db82be8052f1fp-1, 0x1.db412384fff7cp-1, 0x1.daffbeb35e0dap-1, 0x1.dabe8fb9aab37p-1, 0x1.da7d9646d672dp-1, 0x1.da3cd20a82dd5p-1,
    0x1.d9fc42b500930p-1, 0x1.d9bbe7f74d4ebp-1, 0x1.d97bc18311fa4p-1, 0x1.d93bcf0aa0c89p-1, 0x1.d8fc1040f3571p-1, 0x1.d8bc84d9a8d53p-1, 0x1.d87d2c890432ap-1, 0x1.d83e0703ea53ap-1,
    0x1.d7ff13ffe04b9p-1, 0x1.d7c05333099e0p-1, 0x1.d781c45426852p-1, 0x1.d743671a923eap-1, 0x1.d7053b3e415e5p-1, 0x1.d6c74077c0265p-1, 0x1.d689768030e4fp-1, 0x1.d64bdd114a585p-1,
    0x1.d60e73e556174p-1, 0x1.d5d13ab72effbp-1, 0x1.d59431423faa3p-1, 0x1.d557574280e2ep-1, 0x1.d51aac7478279p-1, 0x1.d4de3095362a9p-1, 0x1.d4a1e362555aep-1, 0x1.d465c499f8712p-1,
    0x1.d429d3fac9014p-1, 0x1.d3ee1143f610fp-1, 0x1.d3b27c3532b2fp-1, 0x1.d377148eb4a6dp-1, 0x1.d33bda1132fd2p-1, 0x1.d300cc7de4c07p-1, 0x1.d2c5eb967fa22p-1, 0x1.d28b371d36ac5p-1,
    0x1.d250aed4b8f74p-1, 0x1.d216528030636p-1, 0x1.d1dc21e34057bp-1, 0x1.d1a21cc204836p-1, 0x1.d16842e10fa49p-1, 0x1.d12e94056a51ep-1, 0x1.d0f50ff491c8ep-1, 0x1.d0bbb67476bfep-1,
    0x1.d082874b7c3b3p-1, 0x1.d04982407666ep-1, 0x1.d010a71aa973ap-1, 0x1.cfd7f5a1c8774p-1, 0x1.cf9f6d9df4512p-1, 0x1.cf670ed7ba918p-1, 0x1.cf2ed91814650p-1, 0x1.cef6cc286582ep-1,
    0x1.cebee7d27b1f3p-1, 0x1.ce872be08adfap-1, 0x1.ce4f981d31d44p-1, 0x1.ce182c5373732p-1, 0x1.cde0e84eb896fp-1, 0x1.cda9cbdace813p-1, 0x1.cd72d6c3e5defp-1, 0x1.cd3c08d691d12p-1,
    0x1.cd0561dfc6f76p-1, 0x1.cccee1acda7e1p-1, 0x1.cc98880b812f3p-1, 0x1.cc6254c9ce85dp-1, 0x1.cc2c47b633c53p-1, 0x1.cbf6609f7f117p-1, 0x1.cbc09f54da8c0p-1, 0x1.cb8b03a5cb724p-1,
    0x1.cb558d62313eep-1, 0x1.cb203c5a44ce1p-1, 0x1.caeb105e9783fp-1, 0x1.cab609401275dp-1, 0x1.ca8126cff595fp-1, 0x1.ca4c68dfd6e1bp-1, 0x1.ca17cf41a191ep-1, 0x1.c9e359c7954e5p-1,
    0x1.c9af08444562cp-1, 0x1.c97ada8a97f6cp-1, 0x1.c946d06dc5480p-1, 0x1.c912e9c156e64p-1, 0x1.c8df265926f24p-1, 0x1.c8ab86095f5e7p-1, 0x1.c87808a67931fp-1, 0x1.c844ae053bcdcp-1,
    0x1.c81175fabc340p-1, 0x1.c7de605c5c515p-1, 0x1.c7ab6cffca489p-1, 0x1.c7789bbaffc00p-1, 0x1.c745ec6441310p-1, 0x1.c7135ed21d397p-1, 0x1.c6e0f2db6bef8p-1, 0x1.c6aea8574e36cp-1,
    0x1.c67c7f1d2d178p-1, 0x1.c64a7704b9186p-1, 0x1.c6188fe5e998ep-1, 0x1.c5e6c998fc2edp-1, 0x1.c5b523f67404ap-1, 0x1.c5839ed7193a2p-1, 0x1.c5523a13f846bp-1, 0x1.c520f586615d5p-1,
    0x1.c4efd107e7d27p-1, 0x1.c4becc7261831p-1, 0x1.c48de79fe63e5p-1, 0x1.c45d226acf2fep-1, 0x1.c42c7cadb64c6p-1, 0x1.c3fbf64375bfcp-1, 0x1.c3cb8f07275c4p-1, 0x1.c39b46d4240c1p-1,
    0x1.c36b1d860343fp-1, 0x1.c33b12f89a773p-1, 0x1.c30b2707fc8dap-1, 0x1.c2db5990795acp-1, 0x1.c2abaa6e9d167p-1, 0x1.c27c197f2fd70p-1, 0x1.c24ca69f350cfp-1, 0x1.c21d51abeaffcp-1,
    0x1.c1ee1a82ca4c7p-1, 0x1.c1bf010185652p-1, 0x1.c190050608127p-1, 0x1.c161266e76f5cp-1, 0x1.c13265192f0cfp-1, 0x1.c103c0e4c537dp-1, 0x1.c0d539b005be1p-1, 0x1.c0a6cf59f3d74p-1,
    0x1.c07881c1c933bp-1, 0x1.c04a50c6f586ap-1, 0x1.c01c3c491e117p-1, 0x1.bfee44281d30ap-1, 0x1.bfc0684401e95p-1, 0x1.bf92a87d0f789p-1, 0x1.bf6504b3bce32p-1, 0x1.bf377cc8b4876p-1,
    0x1.bf0a109cd3af1p-1, 0x1.bedcc0112a23bp-1, 0x1.beaf8b06f9c29p-1, 0x1.be82715fb6130p-1, 0x1.be5572fd03dd3p-1, 0x1.be288fc0b8c20p-1, 0x1.bdfbc78cdad43p-1, 0x1.bdcf1a43a0326p-1,
};
static const double __attribute__((aligned(16))) IM_TAB[256] = {
    0x1.ff007fc01ff00p-1, 0x1.fd04794a10e6ap-1, 0x1.fb0c610d5e939p-1, 0x1.f9182b6813bafp-1, 0x1.f727cce5f530ap-1, 0x1.f53b3a3fa204ep-1, 0x1.f3526859b8cecp-1, 0x1.f16d4c4401f17p-1,
    0x1.ef8bdb389ebadp-1, 0x1.edae0a9b3d3a5p-1, 0x1.ebd3cff850b0cp-1, 0x1.e9fd21044e799p-1, 0x1.e829f39aef509p-1, 0x1.e65a3dbe74d6bp-1, 0x1.e48df596f3394p-1, 0x1.e2c511719ee16p-1,
    0x1.e0ff87c01e100p-1, 0x1.df3d4f17de4dbp-1, 0x1.dd7e5e316d94cp-1, 0x1.dbc2abe7d71d4p-1, 0x1.da0a2f3803b41p-1, 0x1.d854df401d855p-1, 0x1.d6a2b33ef7448p-1, 0x1.d4f3a293769cap-1,
    0x1.d347a4bc01d34p-1, 0x1.d19eb155f08a4p-1, 0x1.cff8c01cff8c0p-1, 0x1.ce55c8eac7900p-1, 0x1.ccb5c3b636e3ap-1, 0x1.cb18a8930de60p-1, 0x1.c97e6fb15e44dp-1, 0x1.c7e7115d0ce95p-1,
    0x1.c65285fd56843p-1, 0x1.c4c0c61456a8ep-1, 0x1.c331ca3e91679p-1, 0x1.c1a58b327f576p-1, 0x1.c01c01c01c01cp-1, 0x1.be9526d0769fap-1, 0x1.bd10f365451b6p-1, 0x1.bb8f609879493p-1,
    0x1.ba10679bd8488p-1, 0x1.b89401b89401cp-1, 0x1.b71a284ee6b34p-1, 0x1.b5a2d4d5b081fp-1, 0x1.b42e00da17007p-1, 0x1.b2bba5ff26a23p-1, 0x1.b14bbdfd760e6p-1, 0x1.afde42a2cb482p-1,
    0x1.ae732dd1c2a09p-1, 0x1.ad0a798177693p-1, 0x1.aba41fbd2e5b1p-1, 0x1.aa401aa401aa4p-1, 0x1.a8de64688ebabp-1, 0x1.a77ef750a56dap-1, 0x1.a621cdb4f8fdfp-1, 0x1.a4c6e200d2637p-1,
    0x1.a36e2eb1c432dp-1, 0x1.a217ae575ff2fp-1, 0x1.a0c35b92ecdf1p-1, 0x1.9f713117200d0p-1, 0x1.9e2129a7d5f0ap-1, 0x1.9cd34019cd340p-1, 0x1.9b876f5262dd1p-1, 0x1.9a3db2474fb98p-1,
    0x1.98f603fe670a0p-1, 0x1.97b05f8d56652p-1, 0x1.966cc01966cc0p-1, 0x1.952b20d73ee97p-1, 0x1.93eb7d0aa6759p-1, 0x1.92add0064ab74p-1, 0x1.9172152b841ddp-1, 0x1.903847ea1cec1p-1,
    0x1.8f0063c018f00p-1, 0x1.8dca64397e408p-1, 0x1.8c9644f01efbcp-1, 0x1.8b64018b64019p-1, 0x1.8a3395c018a34p-1, 0x1.8904fd503744bp-1, 0x1.87d8340ab6e97p-1, 0x1.86ad35cb59a84p-1,
    0x1.8583fe7a7c018p-1, 0x1.845c8a0ce5129p-1, 0x1.8336d48397a24p-1, 0x1.8212d9eba4018p-1, 0x1.80f0965dfabcbp-1, 0x1.7fd005ff40180p-1, 0x1.7eb124ffa053bp-1, 0x1.7d93ef9aa4b46p-1,
    0x1.7c7862170949fp-1, 0x1.7b5e78c693733p-1, 0x1.7a463005e918cp-1, 0x1.792f843c689c3p-1, 0x1.781a71dc01782p-1, 0x1.7706f5610d8d0p-1, 0x1.75f50b522b17cp-1, 0x1.74e4b040174e5p-1,
    0x1.73d5e0c5899f7p-1, 0x1.72c899870f91fp-1, 0x1.71bcd732e940ap-1, 0x1.70b29680e66fap-1, 0x1.6fa9d43244380p-1, 0x1.6ea28d118b474p-1, 0x1.6d9cbdf26eaefp-1, 0x1.6c9863b1ab429p-1,
    0x1.6b957b34e7803p-1, 0x1.6a94016a94017p-1, 0x1.6993f349cc726p-1, 0x1.68954dd2390bap-1, 0x1.67980e0bf08c7p-1, 0x1.669c31075ab40p-1, 0x1.65a1b3dd13357p-1, 0x1.64a893adcd25fp-1,
    0x1.63b0cda236e1cp-1, 0x1.62ba5eeade65ep-1, 0x1.61c544c0161c5p-1, 0x1.60d17c61da198p-1, 0x1.5fdf0317b5c6fp-1, 0x1.5eedd630a9fb3p-1, 0x1.5dfdf303137b6p-1, 0x1.5d0f56ec91e57p-1,
    0x1.5c21ff51ef005p-1, 0x1.5b35e99f06714p-1, 0x1.5a4b1346add2bp-1, 0x1.596179c29d2cep-1, 0x1.58791a9357ccep-1, 0x1.5791f34015792p-1, 0x1.56ac0156ac015p-1, 0x1.55c7426b79286p-1,
    0x1.54e3b4194ce66p-1, 0x1.5401540154015p-1, 0x1.53201fcb02fb1p-1, 0x1.5240152401524p-1, 0x1.516131c015161p-1, 0x1.508373590ec9cp-1, 0x1.4fa6d7aeb597cp-1, 0x1.4ecb5c86b3d24p-1,
    0x1.4df0ffac83c01p-1, 0x1.4d17bef15cb4ep-1, 0x1.4c3f982c20723p-1, 0x1.4b68893948d1cp-1, 0x1.4a928ffad5b5cp-1, 0x1.49bdaa583b401p-1, 0x1.48e9d63e504d1p-1, 0x1.4817119f3d325p-1,
    0x1.47455a726abf2p-1, 0x1.4674aeb4717e9p-1, 0x1.45a50c670938fp-1, 0x1.44d67190f8b43p-1, 0x1.4408dc3e05b22p-1, 0x1.433c4a7ee52b4p-1, 0x1.4270ba692bc4dp-1, 0x1.41a62a173e821p-1,
    0x1.40dc97a843ae8p-1, 0x1.4014014014014p-1, 0x1.3f4c65072bf74p-1, 0x1.3e85c12a9d651p-1, 0x1.3dc013dc013dcp-1, 0x1.3cfb5b51698ebp-1, 0x1.3c3795c553afbp-1, 0x1.3b74c1769aa5cp-1,
    0x1.3ab2dca869b81p-1, 0x1.39f1e5a22f36ep-1, 0x1.3931daaf8f721p-1, 0x1.3872ba2057e04p-1, 0x1.37b4824872744p-1, 0x1.36f7317fd9212p-1, 0x1.363ac622898b1p-1, 0x1.357f3e9078e5bp-1,
    0x1.34c4992d87fd9p-1, 0x1.340ad461776d3p-1, 0x1.3351ee97dbfc6p-1, 0x1.3299e6401329ap-1, 0x1.31e2b9cd37dc2p-1, 0x1.312c67b6173eep-1, 0x1.3076ee7525c2cp-1, 0x1.2fc24c8874486p-1,
    0x1.2f0e8071a5703p-1, 0x1.2e5b88b5e3104p-1, 0x1.2da963ddd3cfbp-1, 0x1.2cf8107590e67p-1, 0x1.2c478d0c9c013p-1, 0x1.2b97d835d548ep-1, 0x1.2ae8f087718d0p-1, 0x1.2a3ad49af0907p-1,
    0x1.298d830d13780p-1, 0x1.28e0fa7dd35a3p-1, 0x1.2835399057efdp-1, 0x1.278a3eeaee650p-1, 0x1.26e009370049cp-1, 0x1.263697210aa18p-1, 0x1.258de75895121p-1, 0x1.24e5f89029305p-1,
    0x1.243ec97d49eaep-1, 0x1.239858d86b11fp-1, 0x1.22f2a55ce8fc5p-1, 0x1.224dadc900489p-1, 0x1.21a970ddc5ba7p-1, 0x1.2105ed5f1e336p-1, 0x1.20632213b6c6dp-1, 0x1.1fc10dc4fce8bp-1,
    0x1.1f1faf3f16b64p-1, 0x1.1e7f0550db594p-1, 0x1.1ddf0ecbcb841p-1, 0x1.1d3fca840a074p-1, 0x1.1ca13750547fep-1, 0x1.1c035409fc1dfp-1, 0x1.1b661f8cde833p-1, 0x1.1ac998b75eb90p-1,
    0x1.1a2dbe6a5e3e4p-1, 0x1.19928f89362b7p-1, 0x1.18f80af9b06dcp-1, 0x1.185e2fa401186p-1, 0x1.17c4fc72bfcb9p-1, 0x1.172c7052e1316p-1, 0x1.16948a33b08fap-1, 0x1.15fd4906c96f1p-1,
    0x1.1566abc011567p-1, 0x1.14d0b155b19aep-1, 0x1.143b58c01143bp-1, 0x1.13a6a0f9cf01ep-1, 0x1.131288ffbb3b6p-1, 0x1.127f0fd0d2295p-1, 0x1.11ec346e36092p-1, 0x1.1159f5db29606p-1,
    0x1.10c8531d0952ep-1, 0x1.10374b3b480aap-1, 0x1.0fa6dd3f67322p-1, 0x1.0f170834f27fap-1, 0x1.0e87cb297a51ep-1, 0x1.0df9252c8e5e6p-1, 0x1.0d6b154fb86f9p-1, 0x1.0cdd9aa677344p-1,
    0x1.0c50b446391f3p-1, 0x1.0bc4614657569p-1, 0x1.0b38a0c010b39p-1, 0x1.0aad71ce84d16p-1, 0x1.0a22d38eaf2bfp-1, 0x1.0998c51f624d5p-1, 0x1.090f45a1430aap-1, 0x1.08865436c3cf7p-1,
    0x1.07fdf0041ff7cp-1, 0x1.0776182f57386p-1, 0x1.06eecbe029155p-1, 0x1.06680a4010668p-1, 0x1.05e1d27a3ee9cp-1, 0x1.055c23bb98e2ap-1, 0x1.04d6fd32b0c7bp-1, 0x1.04525e0fc2fcbp-1,
    0x1.03ce4584b19a0p-1, 0x1.034ab2c50040dp-1, 0x1.02c7a505cffbfp-1, 0x1.02451b7ddb2d2p-1, 0x1.01c315657186bp-1, 0x1.014191f674111p-1, 0x1.00c0906c513cfp-1, 0x1.0040100401004p-1,
};
static const double G_TAB[26] = {
    0x1.912652b5f7dd1p+2, 0x1.5d3899ced9cedp+2, 0x1.3003be4f397e3p+2, 0x1.08a8fc60e9c36p+2, 0x1.ccccccccccccdp+1, 0x1.912652b5f7dd0p+1,
    0x1.5d3899ced9cedp+1, 0x1.3003be4f397e2p+1, 0x1.08a8fc60e9c36p+1, 0x1.ccccccccccccdp+0, 0x1.912652b5f7dd0p+0, 0x1.5d3899ced9cedp+0,
    0x1.3003be4f397e1p+0, 0x1.08a8fc60e9c36p+0, 0x1.ccccccccccccdp-1, 0x1.912652b5f7dd0p-1, 0x1.5d3899ced9cedp-1, 0x1.3003be4f397e1p-1,
    0x1.08a8fc60e9c35p-1, 0x1.ccccccccccccdp-2, 0x1.912652b5f7dcfp-2, 0x1.5d3899ced9cedp-2, 0x1.3003be4f397e1p-2, 0x1.08a8fc60e9c35p-2,
    0x1.ccccccccccccdp-3, 0x1.912652b5f7dcfp-3,
};

static const double P5C[6] = { 0.99999999999999944, -0.0003898635477582879, 4.5598075752211103e-07, -6.5182440812596796e-10, 1.0163047558970364e-12, -1.5677849444451524e-15 };


static inline double fast_factor09(double err) {
    union { double d; uint64_t b; } u;
    u.d = err;
    uint64_t b = u.b;
    uint64_t e = (b >> 52) & 0x7FF;
    if (e == 0x7FF) return 0.2;   /* +inf */
    if (e == 0)     return 5.0;   /* subnormal */
    int32_t k = (int32_t)e - 1023;
    int32_t ki = k + 14;
    ki = ki < 0 ? 0 : (ki > 25 ? 25 : ki);
    uint32_t mi = (uint32_t)((b >> 44) & 0xFFu);
    u.b = (b & 0x000FFFFFFFFFFFFFULL) | 0x3FF0000000000000ULL;   /* m in [1,2) */
    double s = fma(u.d, IM_TAB[mi], -1.0);
    double t = fma(s, 512.0, s);       /* s * 513 */
    double p = (((P5C[5] * t + P5C[4]) * t + P5C[3]) * t + P5C[2]) * t + P5C[1];
    p = p * t + P5C[0];
    return FM_TAB[mi] * p * G_TAB[ki];
}

static inline void dopri5_step(
    const double y1, const double y2, const double y3, double h,
    double *s1, double *s2, double *s3, double *err_out)
{
    double k1_0, k1_1, k1_2, k2_0, k2_1, k2_2, k3_0, k3_1, k3_2;
    double k4_0, k4_1, k4_2, k5_0, k5_1, k5_2, k6_0, k6_1, k6_2;
    double k7_0, k7_1, k7_2;

    k1_0 = -0.04 * y1 + 1.0e4 * y2 * y3;
    k1_1 = 0.04 * y1 - 1.0e4 * y2 * y3 - 3.0e7 * y2 * y2;
    k1_2 = 3.0e7 * y2 * y2;

    {
        double p = y1 + h * (1.0/5.0) * k1_0;
        double q = y2 + h * (1.0/5.0) * k1_1;
        double r = y3 + h * (1.0/5.0) * k1_2;
        k2_0 = -0.04 * p + 1.0e4 * q * r;
        k2_1 = 0.04 * p - 1.0e4 * q * r - 3.0e7 * q * q;
        k2_2 = 3.0e7 * q * q;
    }
    {
        double p = y1 + h * (3.0/40.0 * k1_0 + 9.0/40.0 * k2_0);
        double q = y2 + h * (3.0/40.0 * k1_1 + 9.0/40.0 * k2_1);
        double r = y3 + h * (3.0/40.0 * k1_2 + 9.0/40.0 * k2_2);
        k3_0 = -0.04 * p + 1.0e4 * q * r;
        k3_1 = 0.04 * p - 1.0e4 * q * r - 3.0e7 * q * q;
        k3_2 = 3.0e7 * q * q;
    }
    {
        double p = y1 + h * (44.0/45.0 * k1_0 - 56.0/15.0 * k2_0 + 32.0/9.0 * k3_0);
        double q = y2 + h * (44.0/45.0 * k1_1 - 56.0/15.0 * k2_1 + 32.0/9.0 * k3_1);
        double r = y3 + h * (44.0/45.0 * k1_2 - 56.0/15.0 * k2_2 + 32.0/9.0 * k3_2);
        k4_0 = -0.04 * p + 1.0e4 * q * r;
        k4_1 = 0.04 * p - 1.0e4 * q * r - 3.0e7 * q * q;
        k4_2 = 3.0e7 * q * q;
    }
    {
        double p = y1 + h * (19372.0/6561.0 * k1_0 - 25360.0/2187.0 * k2_0
                             + 64448.0/6561.0 * k3_0 - 212.0/729.0 * k4_0);
        double q = y2 + h * (19372.0/6561.0 * k1_1 - 25360.0/2187.0 * k2_1
                             + 64448.0/6561.0 * k3_1 - 212.0/729.0 * k4_1);
        double r = y3 + h * (19372.0/6561.0 * k1_2 - 25360.0/2187.0 * k2_2
                             + 64448.0/6561.0 * k3_2 - 212.0/729.0 * k4_2);
        k5_0 = -0.04 * p + 1.0e4 * q * r;
        k5_1 = 0.04 * p - 1.0e4 * q * r - 3.0e7 * q * q;
        k5_2 = 3.0e7 * q * q;
    }
    {
        double p = y1 + h * (9017.0/3168.0 * k1_0 - 355.0/33.0 * k2_0
                             + 46732.0/5247.0 * k3_0 + 49.0/176.0 * k4_0
                             - 5103.0/18656.0 * k5_0);
        double q = y2 + h * (9017.0/3168.0 * k1_1 - 355.0/33.0 * k2_1
                             + 46732.0/5247.0 * k3_1 + 49.0/176.0 * k4_1
                             - 5103.0/18656.0 * k5_1);
        double r = y3 + h * (9017.0/3168.0 * k1_2 - 355.0/33.0 * k2_2
                             + 46732.0/5247.0 * k3_2 + 49.0/176.0 * k4_2
                             - 5103.0/18656.0 * k5_2);
        k6_0 = -0.04 * p + 1.0e4 * q * r;
        k6_1 = 0.04 * p - 1.0e4 * q * r - 3.0e7 * q * q;
        k6_2 = 3.0e7 * q * q;
    }
    double s1v = y1 + h * (35.0/384.0 * k1_0 + 500.0/1113.0 * k3_0
                           + 125.0/192.0 * k4_0 - 2187.0/6784.0 * k5_0
                           + 11.0/84.0 * k6_0);
    double s2v = y2 + h * (35.0/384.0 * k1_1 + 500.0/1113.0 * k3_1
                           + 125.0/192.0 * k4_1 - 2187.0/6784.0 * k5_1
                           + 11.0/84.0 * k6_1);
    double s3v = y3 + h * (35.0/384.0 * k1_2 + 500.0/1113.0 * k3_2
                           + 125.0/192.0 * k4_2 - 2187.0/6784.0 * k5_2
                           + 11.0/84.0 * k6_2);
    k7_0 = -0.04 * s1v + 1.0e4 * s2v * s3v;
    k7_1 = 0.04 * s1v - 1.0e4 * s2v * s3v - 3.0e7 * s2v * s2v;
    k7_2 = 3.0e7 * s2v * s2v;
    double y4_1 = y1 + h * (5179.0/57600.0 * k1_0 + 7571.0/16695.0 * k3_0
                            + 393.0/640.0 * k4_0 - 92097.0/339200.0 * k5_0
                            + 187.0/2100.0 * k6_0 + 1.0/40.0 * k7_0);
    double y4_2 = y2 + h * (5179.0/57600.0 * k1_1 + 7571.0/16695.0 * k3_1
                            + 393.0/640.0 * k4_1 - 92097.0/339200.0 * k5_1
                            + 187.0/2100.0 * k6_1 + 1.0/40.0 * k7_1);
    double y4_3 = y3 + h * (5179.0/57600.0 * k1_2 + 7571.0/16695.0 * k3_2
                            + 393.0/640.0 * k4_2 - 92097.0/339200.0 * k5_2
                            + 187.0/2100.0 * k6_2 + 1.0/40.0 * k7_2);
    double e1 = s1v - y4_1, e2 = s2v - y4_2, e3 = s3v - y4_3;
    double enorm = sqrt(e1 * e1 + e2 * e2 + e3 * e3);
    double ynorm = sqrt(s1v * s1v + s2v * s2v + s3v * s3v);
    *s1 = s1v; *s2 = s2v; *s3 = s3v;
    *err_out = enorm / (ATOL + RTOL * ynorm);
}

#include <immintrin.h>

/* Four independent systems, one 256-bit lane each. Divergence is      */
/* masked: a finished lane does zero-count, zero-update work.         */
/* ------------------------------------------------------------------ */
static void eight_systems(const double *restrict y0, double *restrict y,
                         int64_t *restrict na, int64_t *restrict nr, int64_t n0, double t_end)
{
    const int64_t base = 3 * n0;
    const __m256i IV = _mm256_set_epi32(21, 18, 15, 12, 9, 6, 3, 0);
    const double *pb = y0 + base;
    __m512d y1 = _mm512_i32gather_pd(IV, pb, 8);
    __m512d y2 = _mm512_i32gather_pd(IV, pb + 1, 8);
    __m512d y3 = _mm512_i32gather_pd(IV, pb + 2, 8);

    const __m512d VTEN = _mm512_set1_pd(t_end);
    __m512d h  = _mm512_set1_pd(H0);
    __m512d t  = _mm512_setzero_pd();
    __m512i steps = _mm512_setzero_si512();
    __m512i accv  = _mm512_setzero_si512();
    __m512i rejv  = _mm512_setzero_si512();
    const __m512i MAXV = _mm512_set1_epi64(MAX_STEPS);
    const __m512i ONE  = _mm512_set1_epi64(1);
    const __m512d V1   = _mm512_set1_pd(1.0);
    const __m512d V0   = _mm512_setzero_pd();
    const __m512d V5   = _mm512_set1_pd(5.0);
    const __m512d VC02 = _mm512_set1_pd(0.2);
    const __m512d VNSGN = _mm512_castsi512_pd(_mm512_set1_epi64(0x8000000000000000LL));

    const __m512d C1E4  = _mm512_set1_pd(1.0e4);
    const __m512d C004  = _mm512_set1_pd(0.04);
    const __m512d C3E7  = _mm512_set1_pd(3.0e7);
    const __m512d C2    = _mm512_set1_pd(1.0 / 5.0);
    const __m512d C340  = _mm512_set1_pd(3.0 / 40.0);
    const __m512d C940  = _mm512_set1_pd(9.0 / 40.0);
    const __m512d C4445 = _mm512_set1_pd(44.0 / 45.0);
    const __m512d CM5615= _mm512_set1_pd(-56.0 / 15.0);
    const __m512d C329  = _mm512_set1_pd(32.0 / 9.0);
    const __m512d C19372= _mm512_set1_pd(19372.0 / 6561.0);
    const __m512d CM25360=_mm512_set1_pd(-25360.0 / 2187.0);
    const __m512d C64448= _mm512_set1_pd(64448.0 / 6561.0);
    const __m512d CM212 = _mm512_set1_pd(-212.0 / 729.0);
    const __m512d C9017 = _mm512_set1_pd(9017.0 / 3168.0);
    const __m512d CM355 = _mm512_set1_pd(-355.0 / 33.0);
    const __m512d C46732= _mm512_set1_pd(46732.0 / 5247.0);
    const __m512d C49   = _mm512_set1_pd(49.0 / 176.0);
    const __m512d CM5103= _mm512_set1_pd(-5103.0 / 18656.0);
    const __m512d S1C1  = _mm512_set1_pd(35.0 / 384.0);
    const __m512d S1C3  = _mm512_set1_pd(500.0 / 1113.0);
    const __m512d S1C4  = _mm512_set1_pd(125.0 / 192.0);
    const __m512d S1C5  = _mm512_set1_pd(-2187.0 / 6784.0);
    const __m512d S1C6  = _mm512_set1_pd(11.0 / 84.0);
    const __m512d Y4C1  = _mm512_set1_pd(5179.0 / 57600.0);
    const __m512d Y4C3  = _mm512_set1_pd(7571.0 / 16695.0);
    const __m512d Y4C4  = _mm512_set1_pd(393.0 / 640.0);
    const __m512d Y4C5  = _mm512_set1_pd(-92097.0 / 339200.0);
    const __m512d Y4C6  = _mm512_set1_pd(187.0 / 2100.0);
    const __m512d Y4C7  = _mm512_set1_pd(1.0 / 40.0);
    const __m512d FM1   = _mm512_set1_pd(-1.0);
    const __m512d C512  = _mm512_set1_pd(512.0);
    const __m512d P5C0  = _mm512_set1_pd(P5C[0]);
    const __m512d P5C1  = _mm512_set1_pd(P5C[1]);
    const __m512d P5C2  = _mm512_set1_pd(P5C[2]);
    const __m512d P5C3  = _mm512_set1_pd(P5C[3]);
    const __m512d P5C4  = _mm512_set1_pd(P5C[4]);
    const __m512d P5C5  = _mm512_set1_pd(P5C[5]);
    const __m512i MMASK = _mm512_set1_epi64(0x000FFFFFFFFFFFFFULL);
    const __m512i MONEP0= _mm512_set1_epi64(0x3FF0000000000000ULL);
    const __m512i CFF   = _mm512_set1_epi64(0xFF);
    const __m512i C1009 = _mm512_set1_epi64(1009);
    const __m512i CZERO = _mm512_setzero_si512();
    const __m512d VRTOLV = _mm512_set1_pd(1.0e-6);
    const __m512d VATOLV = _mm512_set1_pd(1.0e-9);
    const __m512i C25   = _mm512_set1_epi64(25);

    for (;;) {
        const __mmask8 actv_m = _mm512_cmplt_pd_mask(t, VTEN)
                              & _mm512_cmpgt_epu64_mask(MAXV, steps);
        if (actv_m == 0) break;

        h = _mm512_mask_blend_pd(_mm512_cmplt_pd_mask(VTEN, _mm512_add_pd(t, h)),
                                 h, _mm512_sub_pd(VTEN, t));

        /* stage 1 */
        __m512d r1t1 = _mm512_mul_pd(C1E4, y2);
        __m512d r1u0 = _mm512_mul_pd(r1t1, y3);
        __m512d r1t2 = _mm512_mul_pd(C3E7, y2);
        __m512d r1t2b = _mm512_mul_pd(r1t2, y2);
        __m512d r1w = _mm512_fmadd_pd(C004, y1, _mm512_sub_pd(V0, r1u0));
        __m512d k1_1 = _mm512_sub_pd(r1w, r1t2b);
        __m512d k1_0 = _mm512_xor_pd(r1w, VNSGN);
        __m512d k1_2 = r1t2b;

        /* stage 2 */
        __m512d ch = _mm512_mul_pd(C2, h);
        __m512d p1 = _mm512_fmadd_pd(ch, k1_0, y1);
        __m512d p2 = _mm512_fmadd_pd(ch, k1_1, y2);
        __m512d p3 = _mm512_fmadd_pd(ch, k1_2, y3);
        __m512d r2t1 = _mm512_mul_pd(C1E4, p2);
        __m512d r2u0 = _mm512_mul_pd(r2t1, p3);
        __m512d r2t2 = _mm512_mul_pd(C3E7, p2);
        __m512d r2t2b = _mm512_mul_pd(r2t2, p2);
        __m512d r2w = _mm512_fmadd_pd(C004, p1, _mm512_sub_pd(V0, r2u0));
        __m512d k2_1 = _mm512_sub_pd(r2w, r2t2b);
        __m512d k2_0 = _mm512_xor_pd(r2w, VNSGN);
        __m512d k2_2 = r2t2b;

        /* stage 3 */
        p1 = _mm512_fmadd_pd(h, _mm512_fmadd_pd(C340, k1_0, _mm512_mul_pd(C940, k2_0)), y1);
        p2 = _mm512_fmadd_pd(h, _mm512_fmadd_pd(C340, k1_1, _mm512_mul_pd(C940, k2_1)), y2);
        p3 = _mm512_fmadd_pd(h, _mm512_fmadd_pd(C340, k1_2, _mm512_mul_pd(C940, k2_2)), y3);
        __m512d r3t1 = _mm512_mul_pd(C1E4, p2);
        __m512d r3u0 = _mm512_mul_pd(r3t1, p3);
        __m512d r3t2 = _mm512_mul_pd(C3E7, p2);
        __m512d r3t2b = _mm512_mul_pd(r3t2, p2);
        __m512d r3w = _mm512_fmadd_pd(C004, p1, _mm512_sub_pd(V0, r3u0));
        __m512d k3_1 = _mm512_sub_pd(r3w, r3t2b);
        __m512d k3_0 = _mm512_xor_pd(r3w, VNSGN);
        __m512d k3_2 = r3t2b;

        /* stage 4 */
        p1 = _mm512_fmadd_pd(h, _mm512_fmadd_pd(C329, k3_0,
             _mm512_fmadd_pd(C4445, k1_0, _mm512_mul_pd(CM5615, k2_0))), y1);
        p2 = _mm512_fmadd_pd(h, _mm512_fmadd_pd(C329, k3_1,
             _mm512_fmadd_pd(C4445, k1_1, _mm512_mul_pd(CM5615, k2_1))), y2);
        p3 = _mm512_fmadd_pd(h, _mm512_fmadd_pd(C329, k3_2,
             _mm512_fmadd_pd(C4445, k1_2, _mm512_mul_pd(CM5615, k2_2))), y3);
        __m512d r4t1 = _mm512_mul_pd(C1E4, p2);
        __m512d r4u0 = _mm512_mul_pd(r4t1, p3);
        __m512d r4t2 = _mm512_mul_pd(C3E7, p2);
        __m512d r4t2b = _mm512_mul_pd(r4t2, p2);
        __m512d r4w = _mm512_fmadd_pd(C004, p1, _mm512_sub_pd(V0, r4u0));
        __m512d k4_1 = _mm512_sub_pd(r4w, r4t2b);
        __m512d k4_0 = _mm512_xor_pd(r4w, VNSGN);
        __m512d k4_2 = r4t2b;

        /* stage 5 */
        p1 = _mm512_fmadd_pd(h, _mm512_fmadd_pd(CM212, k4_0,
             _mm512_fmadd_pd(C64448, k3_0,
             _mm512_fmadd_pd(C19372, k1_0, _mm512_mul_pd(CM25360, k2_0)))), y1);
        p2 = _mm512_fmadd_pd(h, _mm512_fmadd_pd(CM212, k4_1,
             _mm512_fmadd_pd(C64448, k3_1,
             _mm512_fmadd_pd(C19372, k1_1, _mm512_mul_pd(CM25360, k2_1)))), y2);
        p3 = _mm512_fmadd_pd(h, _mm512_fmadd_pd(CM212, k4_2,
             _mm512_fmadd_pd(C64448, k3_2,
             _mm512_fmadd_pd(C19372, k1_2, _mm512_mul_pd(CM25360, k2_2)))), y3);
        __m512d r5t1 = _mm512_mul_pd(C1E4, p2);
        __m512d r5u0 = _mm512_mul_pd(r5t1, p3);
        __m512d r5t2 = _mm512_mul_pd(C3E7, p2);
        __m512d r5t2b = _mm512_mul_pd(r5t2, p2);
        __m512d r5w = _mm512_fmadd_pd(C004, p1, _mm512_sub_pd(V0, r5u0));
        __m512d k5_1 = _mm512_sub_pd(r5w, r5t2b);
        __m512d k5_0 = _mm512_xor_pd(r5w, VNSGN);
        __m512d k5_2 = r5t2b;

        /* stage 6 */
        p1 = _mm512_fmadd_pd(h, _mm512_fmadd_pd(CM5103, k5_0,
             _mm512_fmadd_pd(C49, k4_0,
             _mm512_fmadd_pd(C46732, k3_0,
             _mm512_fmadd_pd(C9017, k1_0, _mm512_mul_pd(CM355, k2_0))))), y1);
        p2 = _mm512_fmadd_pd(h, _mm512_fmadd_pd(CM5103, k5_1,
             _mm512_fmadd_pd(C49, k4_1,
             _mm512_fmadd_pd(C46732, k3_1,
             _mm512_fmadd_pd(C9017, k1_1, _mm512_mul_pd(CM355, k2_1))))), y2);
        p3 = _mm512_fmadd_pd(h, _mm512_fmadd_pd(CM5103, k5_2,
             _mm512_fmadd_pd(C49, k4_2,
             _mm512_fmadd_pd(C46732, k3_2,
             _mm512_fmadd_pd(C9017, k1_2, _mm512_mul_pd(CM355, k2_2))))), y3);
        __m512d r6t1 = _mm512_mul_pd(C1E4, p2);
        __m512d r6u0 = _mm512_mul_pd(r6t1, p3);
        __m512d r6t2 = _mm512_mul_pd(C3E7, p2);
        __m512d r6t2b = _mm512_mul_pd(r6t2, p2);
        __m512d r6w = _mm512_fmadd_pd(C004, p1, _mm512_sub_pd(V0, r6u0));
        __m512d k6_1 = _mm512_sub_pd(r6w, r6t2b);
        __m512d k6_0 = _mm512_xor_pd(r6w, VNSGN);
        __m512d k6_2 = r6t2b;

        /* order-5 sum partials (shared by s and y4) */
        __m512d w1_0 = _mm512_fmadd_pd(S1C1, k1_0, _mm512_mul_pd(S1C3, k3_0));
        __m512d w1_1 = _mm512_fmadd_pd(S1C1, k1_1, _mm512_mul_pd(S1C3, k3_1));
        __m512d w1_2 = _mm512_fmadd_pd(S1C1, k1_2, _mm512_mul_pd(S1C3, k3_2));
        __m512d w2_0 = _mm512_fmadd_pd(S1C4, k4_0, w1_0);
        __m512d w2_1 = _mm512_fmadd_pd(S1C4, k4_1, w1_1);
        __m512d w2_2 = _mm512_fmadd_pd(S1C4, k4_2, w1_2);
        __m512d w3_0 = _mm512_fmadd_pd(S1C5, k5_0, w2_0);
        __m512d w3_1 = _mm512_fmadd_pd(S1C5, k5_1, w2_1);
        __m512d w3_2 = _mm512_fmadd_pd(S1C5, k5_2, w2_2);
        __m512d s1v = _mm512_fmadd_pd(h, _mm512_fmadd_pd(S1C6, k6_0, w3_0), y1);
        __m512d s2v = _mm512_fmadd_pd(h, _mm512_fmadd_pd(S1C6, k6_1, w3_1), y2);
        __m512d s3v = _mm512_fmadd_pd(h, _mm512_fmadd_pd(S1C6, k6_2, w3_2), y3);

        /* stage 7 (FSAL) */
        __m512d r7t1 = _mm512_mul_pd(C1E4, s2v);
        __m512d r7u0 = _mm512_mul_pd(r7t1, s3v);
        __m512d r7t2 = _mm512_mul_pd(C3E7, s2v);
        __m512d r7t2b = _mm512_mul_pd(r7t2, s2v);
        __m512d r7w = _mm512_fmadd_pd(C004, s1v, _mm512_sub_pd(V0, r7u0));
        __m512d k7_1 = _mm512_sub_pd(r7w, r7t2b);
        __m512d k7_0 = _mm512_xor_pd(r7w, VNSGN);
        __m512d k7_2 = r7t2b;

        /* order-4 candidate */
        __m512d v1_0 = _mm512_fmadd_pd(Y4C1, k1_0, _mm512_mul_pd(Y4C3, k3_0));
        __m512d v1_1 = _mm512_fmadd_pd(Y4C1, k1_1, _mm512_mul_pd(Y4C3, k3_1));
        __m512d v1_2 = _mm512_fmadd_pd(Y4C1, k1_2, _mm512_mul_pd(Y4C3, k3_2));
        __m512d v2_0 = _mm512_fmadd_pd(Y4C4, k4_0, v1_0);
        __m512d v2_1 = _mm512_fmadd_pd(Y4C4, k4_1, v1_1);
        __m512d v2_2 = _mm512_fmadd_pd(Y4C4, k4_2, v1_2);
        __m512d v3_0 = _mm512_fmadd_pd(Y4C5, k5_0, v2_0);
        __m512d v3_1 = _mm512_fmadd_pd(Y4C5, k5_1, v2_1);
        __m512d v3_2 = _mm512_fmadd_pd(Y4C5, k5_2, v2_2);
        __m512d v4_0 = _mm512_fmadd_pd(Y4C6, k6_0, v3_0);
        __m512d v4_1 = _mm512_fmadd_pd(Y4C6, k6_1, v3_1);
        __m512d v4_2 = _mm512_fmadd_pd(Y4C6, k6_2, v3_2);
        __m512d y4_0 = _mm512_fmadd_pd(h, _mm512_fmadd_pd(Y4C7, k7_0, v4_0), y1);
        __m512d y4_1 = _mm512_fmadd_pd(h, _mm512_fmadd_pd(Y4C7, k7_1, v4_1), y2);
        __m512d y4_2 = _mm512_fmadd_pd(h, _mm512_fmadd_pd(Y4C7, k7_2, v4_2), y3);

        /* local error */
        __m512d e0 = _mm512_sub_pd(s1v, y4_0);
        __m512d e1 = _mm512_sub_pd(s2v, y4_1);
        __m512d e2 = _mm512_sub_pd(s3v, y4_2);
        __m512d enorm = _mm512_sqrt_pd(_mm512_fmadd_pd(e2, e2,
                        _mm512_fmadd_pd(e0, e0, _mm512_mul_pd(e1, e1))));
        __m512d ynorm = _mm512_sqrt_pd(_mm512_fmadd_pd(s3v, s3v,
                        _mm512_fmadd_pd(s1v, s1v, _mm512_mul_pd(s2v, s2v))));
        __m512d err = _mm512_div_pd(enorm, _mm512_fmadd_pd(VRTOLV, ynorm, VATOLV));

        /* fast factor 0.9*err^(-1/5), clamped, with err==0 -> 5 */
        __m512i bi = _mm512_castpd_si512(err);
        __m512i e64 = _mm512_srli_epi64(bi, 52);
        __m512i m64 = _mm512_or_si512(_mm512_and_si512(bi, MMASK), MONEP0);
        __m512i mi64 = _mm512_and_si512(_mm512_srli_epi64(bi, 44), CFF);
        __m512i ki64 = _mm512_min_epi64(_mm512_max_epi64(_mm512_sub_epi64(e64, C1009), CZERO), C25);
        __m512d fm = _mm512_i64gather_pd(mi64, FM_TAB, 8);
        __m512d im = _mm512_i64gather_pd(mi64, IM_TAB, 8);
        __m512d gt = _mm512_i64gather_pd(ki64, G_TAB, 8);
        __m512d mv = _mm512_castsi512_pd(m64);
        __m512d u  = _mm512_fmadd_pd(mv, im, FM1);
        u = _mm512_fmadd_pd(u, C512, u);
        __m512d pp = _mm512_fmadd_pd(P5C5, u, P5C4);
        pp = _mm512_fmadd_pd(pp, u, P5C3);
        pp = _mm512_fmadd_pd(pp, u, P5C2);
        pp = _mm512_fmadd_pd(pp, u, P5C1);
        pp = _mm512_fmadd_pd(pp, u, P5C0);
        __m512d raw = _mm512_mul_pd(_mm512_mul_pd(fm, pp), gt);
        __m512d factor = _mm512_mask_blend_pd(_mm512_cmplt_pd_mask(V0, err), V5, raw);
        factor = _mm512_min_pd(_mm512_max_pd(factor, VC02), V5);

        /* accept / reject / advance (masked) */
        const __mmask8 accm_m = _mm512_cmple_pd_mask(err, V1) & actv_m;
        y1 = _mm512_mask_blend_pd(accm_m, y1, s1v);
        y2 = _mm512_mask_blend_pd(accm_m, y2, s2v);
        y3 = _mm512_mask_blend_pd(accm_m, y3, s3v);
        t = _mm512_mask_add_pd(t, accm_m, t, h);
        accv = _mm512_mask_add_epi64(accv, accm_m, accv, ONE);
        rejv = _mm512_mask_add_epi64(rejv, _mm512_cmplt_pd_mask(V1, err) & actv_m, rejv, ONE);
        steps = _mm512_mask_add_epi64(steps, actv_m, steps, ONE);
        h = _mm512_mul_pd(h, factor);
    }

    double tmp[8];
    _mm512_storeu_pd(tmp, y1);
    for (int l = 0; l < 8; l++) y[base + 3 * l] = tmp[l];
    _mm512_storeu_pd(tmp, y2);
    for (int l = 0; l < 8; l++) y[base + 3 * l + 1] = tmp[l];
    _mm512_storeu_pd(tmp, y3);
    for (int l = 0; l < 8; l++) y[base + 3 * l + 2] = tmp[l];
    int64_t ti[8];
    _mm512_storeu_si512(ti, accv);
    for (int l = 0; l < 8; l++) na[n0 + l] = ti[l];
    _mm512_storeu_si512(ti, rejv);
    for (int l = 0; l < 8; l++) nr[n0 + l] = ti[l];
}
/* ------------------------------------------------------------------ */

static void one_system(const double *restrict y0, double *restrict y,
                       int64_t *restrict na, int64_t *restrict nr, int64_t n, double t_end)
{
    double y1 = y0[3 * n], y2 = y0[3 * n + 1], y3 = y0[3 * n + 2];
    double t = 0.0, h = H0;
    int64_t accepts = 0, rejects = 0, steps = 0;
    while (t < t_end && steps < MAX_STEPS) {
        if (t + h > t_end) h = t_end - t;
        double s1v, s2v, s3v, err;
        dopri5_step(y1, y2, y3, h, &s1v, &s2v, &s3v, &err);
        steps = steps + 1;
        if (err <= 1.0) {
            y1 = s1v; y2 = s2v; y3 = s3v;
            t = t + h;
            accepts = accepts + 1;
        } else {
            rejects = rejects + 1;
        }
        double factor;
        if (err > 0.0) {
            factor = fast_factor09(err);
            if (factor > 5.0) factor = 5.0;
            if (factor < 0.2) factor = 0.2;
        } else {
            factor = 5.0;
        }
        h = h * factor;
    }
    y[3 * n] = y1; y[3 * n + 1] = y2; y[3 * n + 2] = y3;
    na[n] = accepts; nr[n] = rejects;
}

void rk45_ensemble_fp64(int64_t *restrict n_accept, int64_t *restrict n_reject,
                        double *restrict y, const double *restrict y0, const int64_t NSYS,
                        const double t_end, uint8_t *restrict workspace, const int64_t workspace_size)
{
    (void)workspace; (void)workspace_size;
    const int64_t nb = NSYS >> 3;
    #pragma omp parallel for schedule(static)
    for (int64_t b = 0; b < nb; b++)
        eight_systems(y0, y, n_accept, n_reject, 8 * b, t_end);
    for (int64_t n = 8 * nb; n < NSYS; n++)
        one_system(y0, y, n_accept, n_reject, n, t_end);
}
