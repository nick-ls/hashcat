/**
 * Author......: See docs/credits.txt
 * License.....: MIT
 */

#define _SHA1_

#include "inc_vendor.cl"
#include "inc_hash_constants.h"
#include "inc_hash_functions.cl"
#include "inc_types.cl"
#include "inc_common.cl"

#define COMPARE_S "inc_comp_single.cl"
#define COMPARE_M "inc_comp_multi.cl"

__constant u32 c_pc_dec[1024] =
{
  0x00000030,
  0x00000031,
  0x00000032,
  0x00000033,
  0x00000034,
  0x00000035,
  0x00000036,
  0x00000037,
  0x00000038,
  0x00000039,
  0x00003031,
  0x00003131,
  0x00003231,
  0x00003331,
  0x00003431,
  0x00003531,
  0x00003631,
  0x00003731,
  0x00003831,
  0x00003931,
  0x00003032,
  0x00003132,
  0x00003232,
  0x00003332,
  0x00003432,
  0x00003532,
  0x00003632,
  0x00003732,
  0x00003832,
  0x00003932,
  0x00003033,
  0x00003133,
  0x00003233,
  0x00003333,
  0x00003433,
  0x00003533,
  0x00003633,
  0x00003733,
  0x00003833,
  0x00003933,
  0x00003034,
  0x00003134,
  0x00003234,
  0x00003334,
  0x00003434,
  0x00003534,
  0x00003634,
  0x00003734,
  0x00003834,
  0x00003934,
  0x00003035,
  0x00003135,
  0x00003235,
  0x00003335,
  0x00003435,
  0x00003535,
  0x00003635,
  0x00003735,
  0x00003835,
  0x00003935,
  0x00003036,
  0x00003136,
  0x00003236,
  0x00003336,
  0x00003436,
  0x00003536,
  0x00003636,
  0x00003736,
  0x00003836,
  0x00003936,
  0x00003037,
  0x00003137,
  0x00003237,
  0x00003337,
  0x00003437,
  0x00003537,
  0x00003637,
  0x00003737,
  0x00003837,
  0x00003937,
  0x00003038,
  0x00003138,
  0x00003238,
  0x00003338,
  0x00003438,
  0x00003538,
  0x00003638,
  0x00003738,
  0x00003838,
  0x00003938,
  0x00003039,
  0x00003139,
  0x00003239,
  0x00003339,
  0x00003439,
  0x00003539,
  0x00003639,
  0x00003739,
  0x00003839,
  0x00003939,
  0x00303031,
  0x00313031,
  0x00323031,
  0x00333031,
  0x00343031,
  0x00353031,
  0x00363031,
  0x00373031,
  0x00383031,
  0x00393031,
  0x00303131,
  0x00313131,
  0x00323131,
  0x00333131,
  0x00343131,
  0x00353131,
  0x00363131,
  0x00373131,
  0x00383131,
  0x00393131,
  0x00303231,
  0x00313231,
  0x00323231,
  0x00333231,
  0x00343231,
  0x00353231,
  0x00363231,
  0x00373231,
  0x00383231,
  0x00393231,
  0x00303331,
  0x00313331,
  0x00323331,
  0x00333331,
  0x00343331,
  0x00353331,
  0x00363331,
  0x00373331,
  0x00383331,
  0x00393331,
  0x00303431,
  0x00313431,
  0x00323431,
  0x00333431,
  0x00343431,
  0x00353431,
  0x00363431,
  0x00373431,
  0x00383431,
  0x00393431,
  0x00303531,
  0x00313531,
  0x00323531,
  0x00333531,
  0x00343531,
  0x00353531,
  0x00363531,
  0x00373531,
  0x00383531,
  0x00393531,
  0x00303631,
  0x00313631,
  0x00323631,
  0x00333631,
  0x00343631,
  0x00353631,
  0x00363631,
  0x00373631,
  0x00383631,
  0x00393631,
  0x00303731,
  0x00313731,
  0x00323731,
  0x00333731,
  0x00343731,
  0x00353731,
  0x00363731,
  0x00373731,
  0x00383731,
  0x00393731,
  0x00303831,
  0x00313831,
  0x00323831,
  0x00333831,
  0x00343831,
  0x00353831,
  0x00363831,
  0x00373831,
  0x00383831,
  0x00393831,
  0x00303931,
  0x00313931,
  0x00323931,
  0x00333931,
  0x00343931,
  0x00353931,
  0x00363931,
  0x00373931,
  0x00383931,
  0x00393931,
  0x00303032,
  0x00313032,
  0x00323032,
  0x00333032,
  0x00343032,
  0x00353032,
  0x00363032,
  0x00373032,
  0x00383032,
  0x00393032,
  0x00303132,
  0x00313132,
  0x00323132,
  0x00333132,
  0x00343132,
  0x00353132,
  0x00363132,
  0x00373132,
  0x00383132,
  0x00393132,
  0x00303232,
  0x00313232,
  0x00323232,
  0x00333232,
  0x00343232,
  0x00353232,
  0x00363232,
  0x00373232,
  0x00383232,
  0x00393232,
  0x00303332,
  0x00313332,
  0x00323332,
  0x00333332,
  0x00343332,
  0x00353332,
  0x00363332,
  0x00373332,
  0x00383332,
  0x00393332,
  0x00303432,
  0x00313432,
  0x00323432,
  0x00333432,
  0x00343432,
  0x00353432,
  0x00363432,
  0x00373432,
  0x00383432,
  0x00393432,
  0x00303532,
  0x00313532,
  0x00323532,
  0x00333532,
  0x00343532,
  0x00353532,
  0x00363532,
  0x00373532,
  0x00383532,
  0x00393532,
  0x00303632,
  0x00313632,
  0x00323632,
  0x00333632,
  0x00343632,
  0x00353632,
  0x00363632,
  0x00373632,
  0x00383632,
  0x00393632,
  0x00303732,
  0x00313732,
  0x00323732,
  0x00333732,
  0x00343732,
  0x00353732,
  0x00363732,
  0x00373732,
  0x00383732,
  0x00393732,
  0x00303832,
  0x00313832,
  0x00323832,
  0x00333832,
  0x00343832,
  0x00353832,
  0x00363832,
  0x00373832,
  0x00383832,
  0x00393832,
  0x00303932,
  0x00313932,
  0x00323932,
  0x00333932,
  0x00343932,
  0x00353932,
  0x00363932,
  0x00373932,
  0x00383932,
  0x00393932,
  0x00303033,
  0x00313033,
  0x00323033,
  0x00333033,
  0x00343033,
  0x00353033,
  0x00363033,
  0x00373033,
  0x00383033,
  0x00393033,
  0x00303133,
  0x00313133,
  0x00323133,
  0x00333133,
  0x00343133,
  0x00353133,
  0x00363133,
  0x00373133,
  0x00383133,
  0x00393133,
  0x00303233,
  0x00313233,
  0x00323233,
  0x00333233,
  0x00343233,
  0x00353233,
  0x00363233,
  0x00373233,
  0x00383233,
  0x00393233,
  0x00303333,
  0x00313333,
  0x00323333,
  0x00333333,
  0x00343333,
  0x00353333,
  0x00363333,
  0x00373333,
  0x00383333,
  0x00393333,
  0x00303433,
  0x00313433,
  0x00323433,
  0x00333433,
  0x00343433,
  0x00353433,
  0x00363433,
  0x00373433,
  0x00383433,
  0x00393433,
  0x00303533,
  0x00313533,
  0x00323533,
  0x00333533,
  0x00343533,
  0x00353533,
  0x00363533,
  0x00373533,
  0x00383533,
  0x00393533,
  0x00303633,
  0x00313633,
  0x00323633,
  0x00333633,
  0x00343633,
  0x00353633,
  0x00363633,
  0x00373633,
  0x00383633,
  0x00393633,
  0x00303733,
  0x00313733,
  0x00323733,
  0x00333733,
  0x00343733,
  0x00353733,
  0x00363733,
  0x00373733,
  0x00383733,
  0x00393733,
  0x00303833,
  0x00313833,
  0x00323833,
  0x00333833,
  0x00343833,
  0x00353833,
  0x00363833,
  0x00373833,
  0x00383833,
  0x00393833,
  0x00303933,
  0x00313933,
  0x00323933,
  0x00333933,
  0x00343933,
  0x00353933,
  0x00363933,
  0x00373933,
  0x00383933,
  0x00393933,
  0x00303034,
  0x00313034,
  0x00323034,
  0x00333034,
  0x00343034,
  0x00353034,
  0x00363034,
  0x00373034,
  0x00383034,
  0x00393034,
  0x00303134,
  0x00313134,
  0x00323134,
  0x00333134,
  0x00343134,
  0x00353134,
  0x00363134,
  0x00373134,
  0x00383134,
  0x00393134,
  0x00303234,
  0x00313234,
  0x00323234,
  0x00333234,
  0x00343234,
  0x00353234,
  0x00363234,
  0x00373234,
  0x00383234,
  0x00393234,
  0x00303334,
  0x00313334,
  0x00323334,
  0x00333334,
  0x00343334,
  0x00353334,
  0x00363334,
  0x00373334,
  0x00383334,
  0x00393334,
  0x00303434,
  0x00313434,
  0x00323434,
  0x00333434,
  0x00343434,
  0x00353434,
  0x00363434,
  0x00373434,
  0x00383434,
  0x00393434,
  0x00303534,
  0x00313534,
  0x00323534,
  0x00333534,
  0x00343534,
  0x00353534,
  0x00363534,
  0x00373534,
  0x00383534,
  0x00393534,
  0x00303634,
  0x00313634,
  0x00323634,
  0x00333634,
  0x00343634,
  0x00353634,
  0x00363634,
  0x00373634,
  0x00383634,
  0x00393634,
  0x00303734,
  0x00313734,
  0x00323734,
  0x00333734,
  0x00343734,
  0x00353734,
  0x00363734,
  0x00373734,
  0x00383734,
  0x00393734,
  0x00303834,
  0x00313834,
  0x00323834,
  0x00333834,
  0x00343834,
  0x00353834,
  0x00363834,
  0x00373834,
  0x00383834,
  0x00393834,
  0x00303934,
  0x00313934,
  0x00323934,
  0x00333934,
  0x00343934,
  0x00353934,
  0x00363934,
  0x00373934,
  0x00383934,
  0x00393934,
  0x00303035,
  0x00313035,
  0x00323035,
  0x00333035,
  0x00343035,
  0x00353035,
  0x00363035,
  0x00373035,
  0x00383035,
  0x00393035,
  0x00303135,
  0x00313135,
  0x00323135,
  0x00333135,
  0x00343135,
  0x00353135,
  0x00363135,
  0x00373135,
  0x00383135,
  0x00393135,
  0x00303235,
  0x00313235,
  0x00323235,
  0x00333235,
  0x00343235,
  0x00353235,
  0x00363235,
  0x00373235,
  0x00383235,
  0x00393235,
  0x00303335,
  0x00313335,
  0x00323335,
  0x00333335,
  0x00343335,
  0x00353335,
  0x00363335,
  0x00373335,
  0x00383335,
  0x00393335,
  0x00303435,
  0x00313435,
  0x00323435,
  0x00333435,
  0x00343435,
  0x00353435,
  0x00363435,
  0x00373435,
  0x00383435,
  0x00393435,
  0x00303535,
  0x00313535,
  0x00323535,
  0x00333535,
  0x00343535,
  0x00353535,
  0x00363535,
  0x00373535,
  0x00383535,
  0x00393535,
  0x00303635,
  0x00313635,
  0x00323635,
  0x00333635,
  0x00343635,
  0x00353635,
  0x00363635,
  0x00373635,
  0x00383635,
  0x00393635,
  0x00303735,
  0x00313735,
  0x00323735,
  0x00333735,
  0x00343735,
  0x00353735,
  0x00363735,
  0x00373735,
  0x00383735,
  0x00393735,
  0x00303835,
  0x00313835,
  0x00323835,
  0x00333835,
  0x00343835,
  0x00353835,
  0x00363835,
  0x00373835,
  0x00383835,
  0x00393835,
  0x00303935,
  0x00313935,
  0x00323935,
  0x00333935,
  0x00343935,
  0x00353935,
  0x00363935,
  0x00373935,
  0x00383935,
  0x00393935,
  0x00303036,
  0x00313036,
  0x00323036,
  0x00333036,
  0x00343036,
  0x00353036,
  0x00363036,
  0x00373036,
  0x00383036,
  0x00393036,
  0x00303136,
  0x00313136,
  0x00323136,
  0x00333136,
  0x00343136,
  0x00353136,
  0x00363136,
  0x00373136,
  0x00383136,
  0x00393136,
  0x00303236,
  0x00313236,
  0x00323236,
  0x00333236,
  0x00343236,
  0x00353236,
  0x00363236,
  0x00373236,
  0x00383236,
  0x00393236,
  0x00303336,
  0x00313336,
  0x00323336,
  0x00333336,
  0x00343336,
  0x00353336,
  0x00363336,
  0x00373336,
  0x00383336,
  0x00393336,
  0x00303436,
  0x00313436,
  0x00323436,
  0x00333436,
  0x00343436,
  0x00353436,
  0x00363436,
  0x00373436,
  0x00383436,
  0x00393436,
  0x00303536,
  0x00313536,
  0x00323536,
  0x00333536,
  0x00343536,
  0x00353536,
  0x00363536,
  0x00373536,
  0x00383536,
  0x00393536,
  0x00303636,
  0x00313636,
  0x00323636,
  0x00333636,
  0x00343636,
  0x00353636,
  0x00363636,
  0x00373636,
  0x00383636,
  0x00393636,
  0x00303736,
  0x00313736,
  0x00323736,
  0x00333736,
  0x00343736,
  0x00353736,
  0x00363736,
  0x00373736,
  0x00383736,
  0x00393736,
  0x00303836,
  0x00313836,
  0x00323836,
  0x00333836,
  0x00343836,
  0x00353836,
  0x00363836,
  0x00373836,
  0x00383836,
  0x00393836,
  0x00303936,
  0x00313936,
  0x00323936,
  0x00333936,
  0x00343936,
  0x00353936,
  0x00363936,
  0x00373936,
  0x00383936,
  0x00393936,
  0x00303037,
  0x00313037,
  0x00323037,
  0x00333037,
  0x00343037,
  0x00353037,
  0x00363037,
  0x00373037,
  0x00383037,
  0x00393037,
  0x00303137,
  0x00313137,
  0x00323137,
  0x00333137,
  0x00343137,
  0x00353137,
  0x00363137,
  0x00373137,
  0x00383137,
  0x00393137,
  0x00303237,
  0x00313237,
  0x00323237,
  0x00333237,
  0x00343237,
  0x00353237,
  0x00363237,
  0x00373237,
  0x00383237,
  0x00393237,
  0x00303337,
  0x00313337,
  0x00323337,
  0x00333337,
  0x00343337,
  0x00353337,
  0x00363337,
  0x00373337,
  0x00383337,
  0x00393337,
  0x00303437,
  0x00313437,
  0x00323437,
  0x00333437,
  0x00343437,
  0x00353437,
  0x00363437,
  0x00373437,
  0x00383437,
  0x00393437,
  0x00303537,
  0x00313537,
  0x00323537,
  0x00333537,
  0x00343537,
  0x00353537,
  0x00363537,
  0x00373537,
  0x00383537,
  0x00393537,
  0x00303637,
  0x00313637,
  0x00323637,
  0x00333637,
  0x00343637,
  0x00353637,
  0x00363637,
  0x00373637,
  0x00383637,
  0x00393637,
  0x00303737,
  0x00313737,
  0x00323737,
  0x00333737,
  0x00343737,
  0x00353737,
  0x00363737,
  0x00373737,
  0x00383737,
  0x00393737,
  0x00303837,
  0x00313837,
  0x00323837,
  0x00333837,
  0x00343837,
  0x00353837,
  0x00363837,
  0x00373837,
  0x00383837,
  0x00393837,
  0x00303937,
  0x00313937,
  0x00323937,
  0x00333937,
  0x00343937,
  0x00353937,
  0x00363937,
  0x00373937,
  0x00383937,
  0x00393937,
  0x00303038,
  0x00313038,
  0x00323038,
  0x00333038,
  0x00343038,
  0x00353038,
  0x00363038,
  0x00373038,
  0x00383038,
  0x00393038,
  0x00303138,
  0x00313138,
  0x00323138,
  0x00333138,
  0x00343138,
  0x00353138,
  0x00363138,
  0x00373138,
  0x00383138,
  0x00393138,
  0x00303238,
  0x00313238,
  0x00323238,
  0x00333238,
  0x00343238,
  0x00353238,
  0x00363238,
  0x00373238,
  0x00383238,
  0x00393238,
  0x00303338,
  0x00313338,
  0x00323338,
  0x00333338,
  0x00343338,
  0x00353338,
  0x00363338,
  0x00373338,
  0x00383338,
  0x00393338,
  0x00303438,
  0x00313438,
  0x00323438,
  0x00333438,
  0x00343438,
  0x00353438,
  0x00363438,
  0x00373438,
  0x00383438,
  0x00393438,
  0x00303538,
  0x00313538,
  0x00323538,
  0x00333538,
  0x00343538,
  0x00353538,
  0x00363538,
  0x00373538,
  0x00383538,
  0x00393538,
  0x00303638,
  0x00313638,
  0x00323638,
  0x00333638,
  0x00343638,
  0x00353638,
  0x00363638,
  0x00373638,
  0x00383638,
  0x00393638,
  0x00303738,
  0x00313738,
  0x00323738,
  0x00333738,
  0x00343738,
  0x00353738,
  0x00363738,
  0x00373738,
  0x00383738,
  0x00393738,
  0x00303838,
  0x00313838,
  0x00323838,
  0x00333838,
  0x00343838,
  0x00353838,
  0x00363838,
  0x00373838,
  0x00383838,
  0x00393838,
  0x00303938,
  0x00313938,
  0x00323938,
  0x00333938,
  0x00343938,
  0x00353938,
  0x00363938,
  0x00373938,
  0x00383938,
  0x00393938,
  0x00303039,
  0x00313039,
  0x00323039,
  0x00333039,
  0x00343039,
  0x00353039,
  0x00363039,
  0x00373039,
  0x00383039,
  0x00393039,
  0x00303139,
  0x00313139,
  0x00323139,
  0x00333139,
  0x00343139,
  0x00353139,
  0x00363139,
  0x00373139,
  0x00383139,
  0x00393139,
  0x00303239,
  0x00313239,
  0x00323239,
  0x00333239,
  0x00343239,
  0x00353239,
  0x00363239,
  0x00373239,
  0x00383239,
  0x00393239,
  0x00303339,
  0x00313339,
  0x00323339,
  0x00333339,
  0x00343339,
  0x00353339,
  0x00363339,
  0x00373339,
  0x00383339,
  0x00393339,
  0x00303439,
  0x00313439,
  0x00323439,
  0x00333439,
  0x00343439,
  0x00353439,
  0x00363439,
  0x00373439,
  0x00383439,
  0x00393439,
  0x00303539,
  0x00313539,
  0x00323539,
  0x00333539,
  0x00343539,
  0x00353539,
  0x00363539,
  0x00373539,
  0x00383539,
  0x00393539,
  0x00303639,
  0x00313639,
  0x00323639,
  0x00333639,
  0x00343639,
  0x00353639,
  0x00363639,
  0x00373639,
  0x00383639,
  0x00393639,
  0x00303739,
  0x00313739,
  0x00323739,
  0x00333739,
  0x00343739,
  0x00353739,
  0x00363739,
  0x00373739,
  0x00383739,
  0x00393739,
  0x00303839,
  0x00313839,
  0x00323839,
  0x00333839,
  0x00343839,
  0x00353839,
  0x00363839,
  0x00373839,
  0x00383839,
  0x00393839,
  0x00303939,
  0x00313939,
  0x00323939,
  0x00333939,
  0x00343939,
  0x00353939,
  0x00363939,
  0x00373939,
  0x00383939,
  0x00393939,
  0x30303031,
  0x31303031,
  0x32303031,
  0x33303031,
  0x34303031,
  0x35303031,
  0x36303031,
  0x37303031,
  0x38303031,
  0x39303031,
  0x30313031,
  0x31313031,
  0x32313031,
  0x33313031,
  0x34313031,
  0x35313031,
  0x36313031,
  0x37313031,
  0x38313031,
  0x39313031,
  0x30323031,
  0x31323031,
  0x32323031,
  0x33323031,
};

__constant u32 c_pc_len[1024] =
{
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  4,
  4,
  4,
  4,
  4,
  4,
  4,
  4,
  4,
  4,
  4,
  4,
  4,
  4,
  4,
  4,
  4,
  4,
  4,
  4,
  4,
  4,
  4,
  4
};

static void append_word (u32 w0[4], u32 w1[4], const u32 append[4], const u32 offset)
{
  switch (offset)
  {
    case 1:
      w0[0] = w0[0]           | append[0] <<  8;
      w0[1] = append[0] >> 24 | append[1] <<  8;
      w0[2] = append[1] >> 24 | append[2] <<  8;
      w0[3] = append[2] >> 24 | append[3] <<  8;
      break;

    case 2:
      w0[0] = w0[0]           | append[0] << 16;
      w0[1] = append[0] >> 16 | append[1] << 16;
      w0[2] = append[1] >> 16 | append[2] << 16;
      w0[3] = append[2] >> 16 | append[3] << 16;
      break;

    case 3:
      w0[0] = w0[0]           | append[0] << 24;
      w0[1] = append[0] >>  8 | append[1] << 24;
      w0[2] = append[1] >>  8 | append[2] << 24;
      w0[3] = append[2] >>  8 | append[3] << 24;
      break;

    case 4:
      w0[1] = append[0];
      w0[2] = append[1];
      w0[3] = append[2];
      w1[0] = append[3];
      break;
  }
}

static void append_salt (u32 w0[4], u32 w1[4], u32 w2[4], const u32 append[5], const u32 offset)
{
  u32 tmp0;
  u32 tmp1;
  u32 tmp2;
  u32 tmp3;
  u32 tmp4;
  u32 tmp5;

  #if defined IS_AMD || defined IS_GENERIC

  const int offset_minus_4 = 4 - (offset & 3);

  tmp0 = amd_bytealign (append[0],         0, offset_minus_4);
  tmp1 = amd_bytealign (append[1], append[0], offset_minus_4);
  tmp2 = amd_bytealign (append[2], append[1], offset_minus_4);
  tmp3 = amd_bytealign (append[3], append[2], offset_minus_4);
  tmp4 = amd_bytealign (append[4], append[3], offset_minus_4);
  tmp5 = amd_bytealign (        0, append[4], offset_minus_4);

  const u32 mod = offset & 3;

  if (mod == 0)
  {
    tmp0 = tmp1;
    tmp1 = tmp2;
    tmp2 = tmp3;
    tmp3 = tmp4;
    tmp4 = tmp5;
    tmp5 = 0;
  }

  #endif

  #ifdef IS_NV

  const int offset_minus_4 = 4 - (offset & 3);

  const int selector = (0x76543210 >> (offset_minus_4 * 4)) & 0xffff;

  tmp0 = __byte_perm (        0, append[0], selector);
  tmp1 = __byte_perm (append[0], append[1], selector);
  tmp2 = __byte_perm (append[1], append[2], selector);
  tmp3 = __byte_perm (append[2], append[3], selector);
  tmp4 = __byte_perm (append[3], append[4], selector);
  tmp5 = __byte_perm (append[4],         0, selector);

  #endif

  const u32 div = offset / 4;

  switch (div)
  {
    case  0:  w0[0] |= tmp0;
              w0[1]  = tmp1;
              w0[2]  = tmp2;
              w0[3]  = tmp3;
              w1[0]  = tmp4;
              w1[1]  = tmp5;
              break;
    case  1:  w0[1] |= tmp0;
              w0[2]  = tmp1;
              w0[3]  = tmp2;
              w1[0]  = tmp3;
              w1[1]  = tmp4;
              w1[2]  = tmp5;
              break;
    case  2:  w0[2] |= tmp0;
              w0[3]  = tmp1;
              w1[0]  = tmp2;
              w1[1]  = tmp3;
              w1[2]  = tmp4;
              w1[3]  = tmp5;
              break;
    case  3:  w0[3] |= tmp0;
              w1[0]  = tmp1;
              w1[1]  = tmp2;
              w1[2]  = tmp3;
              w1[3]  = tmp4;
              w2[0]  = tmp5;
              break;
    case  4:  w1[0] |= tmp0;
              w1[1]  = tmp1;
              w1[2]  = tmp2;
              w1[3]  = tmp3;
              w2[0]  = tmp4;
              w2[1]  = tmp5;
              break;
  }
}

static void sha1_transform (const u32 w0[4], const u32 w1[4], const u32 w2[4], const u32 w3[4], u32 digest[5])
{
  u32 A = digest[0];
  u32 B = digest[1];
  u32 C = digest[2];
  u32 D = digest[3];
  u32 E = digest[4];

  u32 w0_t = w0[0];
  u32 w1_t = w0[1];
  u32 w2_t = w0[2];
  u32 w3_t = w0[3];
  u32 w4_t = w1[0];
  u32 w5_t = w1[1];
  u32 w6_t = w1[2];
  u32 w7_t = w1[3];
  u32 w8_t = w2[0];
  u32 w9_t = w2[1];
  u32 wa_t = w2[2];
  u32 wb_t = w2[3];
  u32 wc_t = w3[0];
  u32 wd_t = w3[1];
  u32 we_t = w3[2];
  u32 wf_t = w3[3];

  #undef K
  #define K SHA1C00

  SHA1_STEP (SHA1_F0o, A, B, C, D, E, w0_t);
  SHA1_STEP (SHA1_F0o, E, A, B, C, D, w1_t);
  SHA1_STEP (SHA1_F0o, D, E, A, B, C, w2_t);
  SHA1_STEP (SHA1_F0o, C, D, E, A, B, w3_t);
  SHA1_STEP (SHA1_F0o, B, C, D, E, A, w4_t);
  SHA1_STEP (SHA1_F0o, A, B, C, D, E, w5_t);
  SHA1_STEP (SHA1_F0o, E, A, B, C, D, w6_t);
  SHA1_STEP (SHA1_F0o, D, E, A, B, C, w7_t);
  SHA1_STEP (SHA1_F0o, C, D, E, A, B, w8_t);
  SHA1_STEP (SHA1_F0o, B, C, D, E, A, w9_t);
  SHA1_STEP (SHA1_F0o, A, B, C, D, E, wa_t);
  SHA1_STEP (SHA1_F0o, E, A, B, C, D, wb_t);
  SHA1_STEP (SHA1_F0o, D, E, A, B, C, wc_t);
  SHA1_STEP (SHA1_F0o, C, D, E, A, B, wd_t);
  SHA1_STEP (SHA1_F0o, B, C, D, E, A, we_t);
  SHA1_STEP (SHA1_F0o, A, B, C, D, E, wf_t);
  w0_t = rotl32 ((wd_t ^ w8_t ^ w2_t ^ w0_t), 1u); SHA1_STEP (SHA1_F0o, E, A, B, C, D, w0_t);
  w1_t = rotl32 ((we_t ^ w9_t ^ w3_t ^ w1_t), 1u); SHA1_STEP (SHA1_F0o, D, E, A, B, C, w1_t);
  w2_t = rotl32 ((wf_t ^ wa_t ^ w4_t ^ w2_t), 1u); SHA1_STEP (SHA1_F0o, C, D, E, A, B, w2_t);
  w3_t = rotl32 ((w0_t ^ wb_t ^ w5_t ^ w3_t), 1u); SHA1_STEP (SHA1_F0o, B, C, D, E, A, w3_t);

  #undef K
  #define K SHA1C01

  w4_t = rotl32 ((w1_t ^ wc_t ^ w6_t ^ w4_t), 1u); SHA1_STEP (SHA1_F1, A, B, C, D, E, w4_t);
  w5_t = rotl32 ((w2_t ^ wd_t ^ w7_t ^ w5_t), 1u); SHA1_STEP (SHA1_F1, E, A, B, C, D, w5_t);
  w6_t = rotl32 ((w3_t ^ we_t ^ w8_t ^ w6_t), 1u); SHA1_STEP (SHA1_F1, D, E, A, B, C, w6_t);
  w7_t = rotl32 ((w4_t ^ wf_t ^ w9_t ^ w7_t), 1u); SHA1_STEP (SHA1_F1, C, D, E, A, B, w7_t);
  w8_t = rotl32 ((w5_t ^ w0_t ^ wa_t ^ w8_t), 1u); SHA1_STEP (SHA1_F1, B, C, D, E, A, w8_t);
  w9_t = rotl32 ((w6_t ^ w1_t ^ wb_t ^ w9_t), 1u); SHA1_STEP (SHA1_F1, A, B, C, D, E, w9_t);
  wa_t = rotl32 ((w7_t ^ w2_t ^ wc_t ^ wa_t), 1u); SHA1_STEP (SHA1_F1, E, A, B, C, D, wa_t);
  wb_t = rotl32 ((w8_t ^ w3_t ^ wd_t ^ wb_t), 1u); SHA1_STEP (SHA1_F1, D, E, A, B, C, wb_t);
  wc_t = rotl32 ((w9_t ^ w4_t ^ we_t ^ wc_t), 1u); SHA1_STEP (SHA1_F1, C, D, E, A, B, wc_t);
  wd_t = rotl32 ((wa_t ^ w5_t ^ wf_t ^ wd_t), 1u); SHA1_STEP (SHA1_F1, B, C, D, E, A, wd_t);
  we_t = rotl32 ((wb_t ^ w6_t ^ w0_t ^ we_t), 1u); SHA1_STEP (SHA1_F1, A, B, C, D, E, we_t);
  wf_t = rotl32 ((wc_t ^ w7_t ^ w1_t ^ wf_t), 1u); SHA1_STEP (SHA1_F1, E, A, B, C, D, wf_t);
  w0_t = rotl32 ((wd_t ^ w8_t ^ w2_t ^ w0_t), 1u); SHA1_STEP (SHA1_F1, D, E, A, B, C, w0_t);
  w1_t = rotl32 ((we_t ^ w9_t ^ w3_t ^ w1_t), 1u); SHA1_STEP (SHA1_F1, C, D, E, A, B, w1_t);
  w2_t = rotl32 ((wf_t ^ wa_t ^ w4_t ^ w2_t), 1u); SHA1_STEP (SHA1_F1, B, C, D, E, A, w2_t);
  w3_t = rotl32 ((w0_t ^ wb_t ^ w5_t ^ w3_t), 1u); SHA1_STEP (SHA1_F1, A, B, C, D, E, w3_t);
  w4_t = rotl32 ((w1_t ^ wc_t ^ w6_t ^ w4_t), 1u); SHA1_STEP (SHA1_F1, E, A, B, C, D, w4_t);
  w5_t = rotl32 ((w2_t ^ wd_t ^ w7_t ^ w5_t), 1u); SHA1_STEP (SHA1_F1, D, E, A, B, C, w5_t);
  w6_t = rotl32 ((w3_t ^ we_t ^ w8_t ^ w6_t), 1u); SHA1_STEP (SHA1_F1, C, D, E, A, B, w6_t);
  w7_t = rotl32 ((w4_t ^ wf_t ^ w9_t ^ w7_t), 1u); SHA1_STEP (SHA1_F1, B, C, D, E, A, w7_t);

  #undef K
  #define K SHA1C02

  w8_t = rotl32 ((w5_t ^ w0_t ^ wa_t ^ w8_t), 1u); SHA1_STEP (SHA1_F2o, A, B, C, D, E, w8_t);
  w9_t = rotl32 ((w6_t ^ w1_t ^ wb_t ^ w9_t), 1u); SHA1_STEP (SHA1_F2o, E, A, B, C, D, w9_t);
  wa_t = rotl32 ((w7_t ^ w2_t ^ wc_t ^ wa_t), 1u); SHA1_STEP (SHA1_F2o, D, E, A, B, C, wa_t);
  wb_t = rotl32 ((w8_t ^ w3_t ^ wd_t ^ wb_t), 1u); SHA1_STEP (SHA1_F2o, C, D, E, A, B, wb_t);
  wc_t = rotl32 ((w9_t ^ w4_t ^ we_t ^ wc_t), 1u); SHA1_STEP (SHA1_F2o, B, C, D, E, A, wc_t);
  wd_t = rotl32 ((wa_t ^ w5_t ^ wf_t ^ wd_t), 1u); SHA1_STEP (SHA1_F2o, A, B, C, D, E, wd_t);
  we_t = rotl32 ((wb_t ^ w6_t ^ w0_t ^ we_t), 1u); SHA1_STEP (SHA1_F2o, E, A, B, C, D, we_t);
  wf_t = rotl32 ((wc_t ^ w7_t ^ w1_t ^ wf_t), 1u); SHA1_STEP (SHA1_F2o, D, E, A, B, C, wf_t);
  w0_t = rotl32 ((wd_t ^ w8_t ^ w2_t ^ w0_t), 1u); SHA1_STEP (SHA1_F2o, C, D, E, A, B, w0_t);
  w1_t = rotl32 ((we_t ^ w9_t ^ w3_t ^ w1_t), 1u); SHA1_STEP (SHA1_F2o, B, C, D, E, A, w1_t);
  w2_t = rotl32 ((wf_t ^ wa_t ^ w4_t ^ w2_t), 1u); SHA1_STEP (SHA1_F2o, A, B, C, D, E, w2_t);
  w3_t = rotl32 ((w0_t ^ wb_t ^ w5_t ^ w3_t), 1u); SHA1_STEP (SHA1_F2o, E, A, B, C, D, w3_t);
  w4_t = rotl32 ((w1_t ^ wc_t ^ w6_t ^ w4_t), 1u); SHA1_STEP (SHA1_F2o, D, E, A, B, C, w4_t);
  w5_t = rotl32 ((w2_t ^ wd_t ^ w7_t ^ w5_t), 1u); SHA1_STEP (SHA1_F2o, C, D, E, A, B, w5_t);
  w6_t = rotl32 ((w3_t ^ we_t ^ w8_t ^ w6_t), 1u); SHA1_STEP (SHA1_F2o, B, C, D, E, A, w6_t);
  w7_t = rotl32 ((w4_t ^ wf_t ^ w9_t ^ w7_t), 1u); SHA1_STEP (SHA1_F2o, A, B, C, D, E, w7_t);
  w8_t = rotl32 ((w5_t ^ w0_t ^ wa_t ^ w8_t), 1u); SHA1_STEP (SHA1_F2o, E, A, B, C, D, w8_t);
  w9_t = rotl32 ((w6_t ^ w1_t ^ wb_t ^ w9_t), 1u); SHA1_STEP (SHA1_F2o, D, E, A, B, C, w9_t);
  wa_t = rotl32 ((w7_t ^ w2_t ^ wc_t ^ wa_t), 1u); SHA1_STEP (SHA1_F2o, C, D, E, A, B, wa_t);
  wb_t = rotl32 ((w8_t ^ w3_t ^ wd_t ^ wb_t), 1u); SHA1_STEP (SHA1_F2o, B, C, D, E, A, wb_t);

  #undef K
  #define K SHA1C03

  wc_t = rotl32 ((w9_t ^ w4_t ^ we_t ^ wc_t), 1u); SHA1_STEP (SHA1_F1, A, B, C, D, E, wc_t);
  wd_t = rotl32 ((wa_t ^ w5_t ^ wf_t ^ wd_t), 1u); SHA1_STEP (SHA1_F1, E, A, B, C, D, wd_t);
  we_t = rotl32 ((wb_t ^ w6_t ^ w0_t ^ we_t), 1u); SHA1_STEP (SHA1_F1, D, E, A, B, C, we_t);
  wf_t = rotl32 ((wc_t ^ w7_t ^ w1_t ^ wf_t), 1u); SHA1_STEP (SHA1_F1, C, D, E, A, B, wf_t);
  w0_t = rotl32 ((wd_t ^ w8_t ^ w2_t ^ w0_t), 1u); SHA1_STEP (SHA1_F1, B, C, D, E, A, w0_t);
  w1_t = rotl32 ((we_t ^ w9_t ^ w3_t ^ w1_t), 1u); SHA1_STEP (SHA1_F1, A, B, C, D, E, w1_t);
  w2_t = rotl32 ((wf_t ^ wa_t ^ w4_t ^ w2_t), 1u); SHA1_STEP (SHA1_F1, E, A, B, C, D, w2_t);
  w3_t = rotl32 ((w0_t ^ wb_t ^ w5_t ^ w3_t), 1u); SHA1_STEP (SHA1_F1, D, E, A, B, C, w3_t);
  w4_t = rotl32 ((w1_t ^ wc_t ^ w6_t ^ w4_t), 1u); SHA1_STEP (SHA1_F1, C, D, E, A, B, w4_t);
  w5_t = rotl32 ((w2_t ^ wd_t ^ w7_t ^ w5_t), 1u); SHA1_STEP (SHA1_F1, B, C, D, E, A, w5_t);
  w6_t = rotl32 ((w3_t ^ we_t ^ w8_t ^ w6_t), 1u); SHA1_STEP (SHA1_F1, A, B, C, D, E, w6_t);
  w7_t = rotl32 ((w4_t ^ wf_t ^ w9_t ^ w7_t), 1u); SHA1_STEP (SHA1_F1, E, A, B, C, D, w7_t);
  w8_t = rotl32 ((w5_t ^ w0_t ^ wa_t ^ w8_t), 1u); SHA1_STEP (SHA1_F1, D, E, A, B, C, w8_t);
  w9_t = rotl32 ((w6_t ^ w1_t ^ wb_t ^ w9_t), 1u); SHA1_STEP (SHA1_F1, C, D, E, A, B, w9_t);
  wa_t = rotl32 ((w7_t ^ w2_t ^ wc_t ^ wa_t), 1u); SHA1_STEP (SHA1_F1, B, C, D, E, A, wa_t);
  wb_t = rotl32 ((w8_t ^ w3_t ^ wd_t ^ wb_t), 1u); SHA1_STEP (SHA1_F1, A, B, C, D, E, wb_t);
  wc_t = rotl32 ((w9_t ^ w4_t ^ we_t ^ wc_t), 1u); SHA1_STEP (SHA1_F1, E, A, B, C, D, wc_t);
  wd_t = rotl32 ((wa_t ^ w5_t ^ wf_t ^ wd_t), 1u); SHA1_STEP (SHA1_F1, D, E, A, B, C, wd_t);
  we_t = rotl32 ((wb_t ^ w6_t ^ w0_t ^ we_t), 1u); SHA1_STEP (SHA1_F1, C, D, E, A, B, we_t);
  wf_t = rotl32 ((wc_t ^ w7_t ^ w1_t ^ wf_t), 1u); SHA1_STEP (SHA1_F1, B, C, D, E, A, wf_t);

  digest[0] += A;
  digest[1] += B;
  digest[2] += C;
  digest[3] += D;
  digest[4] += E;
}

__kernel void m05800_init (__global pw_t *pws, __global const kernel_rule_t *rules_buf, __global const comb_t *combs_buf, __global const bf_t *bfs_buf, __global androidpin_tmp_t *tmps, __global void *hooks, __global const u32 *bitmaps_buf_s1_a, __global const u32 *bitmaps_buf_s1_b, __global const u32 *bitmaps_buf_s1_c, __global const u32 *bitmaps_buf_s1_d, __global const u32 *bitmaps_buf_s2_a, __global const u32 *bitmaps_buf_s2_b, __global const u32 *bitmaps_buf_s2_c, __global const u32 *bitmaps_buf_s2_d, __global plain_t *plains_buf, __global const digest_t *digests_buf, __global u32 *hashes_shown, __global const salt_t *salt_bufs, __global const void *esalt_bufs, __global u32 *d_return_buf, __global u32 *d_scryptV0_buf, __global u32 *d_scryptV1_buf, __global u32 *d_scryptV2_buf, __global u32 *d_scryptV3_buf, const u32 bitmap_mask, const u32 bitmap_shift1, const u32 bitmap_shift2, const u32 salt_pos, const u32 loop_pos, const u32 loop_cnt, const u32 il_cnt, const u32 digests_cnt, const u32 digests_offset, const u32 combs_mode, const u32 gid_max)
{
  /**
   * base
   */

  const u32 gid = get_global_id (0);

  if (gid >= gid_max) return;

  u32 word_buf[4];

  word_buf[0] = pws[gid].i[ 0];
  word_buf[1] = pws[gid].i[ 1];
  word_buf[2] = pws[gid].i[ 2];
  word_buf[3] = pws[gid].i[ 3];

  const u32 pw_len = pws[gid].pw_len;

  /**
   * salt
   */

  u32 salt_len = salt_bufs[salt_pos].salt_len;

  u32 salt_buf[5];

  salt_buf[0] = salt_bufs[salt_pos].salt_buf[0];
  salt_buf[1] = salt_bufs[salt_pos].salt_buf[1];
  salt_buf[2] = salt_bufs[salt_pos].salt_buf[2];
  salt_buf[3] = salt_bufs[salt_pos].salt_buf[3];
  salt_buf[4] = salt_bufs[salt_pos].salt_buf[4];

  /**
   * init
   */

  const u32 pc_len = 1;
  const u32 pc_dec = 0x30;

  u32 data0[4] = { 0, 0, 0, 0 };
  u32 data1[4] = { 0, 0, 0, 0 };
  u32 data2[4] = { 0, 0, 0, 0 };

  data0[0] = pc_dec;

  append_word (data0, data1, word_buf, pc_len);

  append_salt (data0, data1, data2, salt_buf, pc_len + pw_len);

  u32 w0[4];
  u32 w1[4];
  u32 w2[4];
  u32 w3[4];

  w0[0] = swap32 (data0[0]);
  w0[1] = swap32 (data0[1]);
  w0[2] = swap32 (data0[2]);
  w0[3] = swap32 (data0[3]);
  w1[0] = swap32 (data1[0]);
  w1[1] = swap32 (data1[1]);
  w1[2] = swap32 (data1[2]);
  w1[3] = swap32 (data1[3]);
  w2[0] = swap32 (data2[0]);
  w2[1] = swap32 (data2[1]);
  w2[2] = 0;
  w2[3] = 0;
  w3[0] = 0;
  w3[1] = 0;
  w3[2] = 0;
  w3[3] = (pc_len + pw_len + salt_len) * 8;

  u32 digest[5];

  digest[0] = SHA1M_A;
  digest[1] = SHA1M_B;
  digest[2] = SHA1M_C;
  digest[3] = SHA1M_D;
  digest[4] = SHA1M_E;

  sha1_transform (w0, w1, w2, w3, digest);

  tmps[gid].digest_buf[0] = digest[0];
  tmps[gid].digest_buf[1] = digest[1];
  tmps[gid].digest_buf[2] = digest[2];
  tmps[gid].digest_buf[3] = digest[3];
  tmps[gid].digest_buf[4] = digest[4];
}

__kernel void m05800_loop (__global pw_t *pws, __global const kernel_rule_t *rules_buf, __global const comb_t *combs_buf, __global const bf_t *bfs_buf, __global androidpin_tmp_t *tmps, __global void *hooks, __global const u32 *bitmaps_buf_s1_a, __global const u32 *bitmaps_buf_s1_b, __global const u32 *bitmaps_buf_s1_c, __global const u32 *bitmaps_buf_s1_d, __global const u32 *bitmaps_buf_s2_a, __global const u32 *bitmaps_buf_s2_b, __global const u32 *bitmaps_buf_s2_c, __global const u32 *bitmaps_buf_s2_d, __global plain_t *plains_buf, __global const digest_t *digests_buf, __global u32 *hashes_shown, __global const salt_t *salt_bufs, __global const void *esalt_bufs, __global u32 *d_return_buf, __global u32 *d_scryptV0_buf, __global u32 *d_scryptV1_buf, __global u32 *d_scryptV2_buf, __global u32 *d_scryptV3_buf, const u32 bitmap_mask, const u32 bitmap_shift1, const u32 bitmap_shift2, const u32 salt_pos, const u32 loop_pos, const u32 loop_cnt, const u32 il_cnt, const u32 digests_cnt, const u32 digests_offset, const u32 combs_mode, const u32 gid_max)
{
  /**
   * base
   */

  const u32 gid = get_global_id (0);
  const u32 lid = get_local_id (0);
  const u32 lsz = get_local_size (0);

  /**
   * cache precomputed conversion table in shared memory
   */

  __local u32 s_pc_dec[1024];
  __local u32 s_pc_len[1024];

  for (u32 i = lid; i < 1024; i += lsz)
  {
    s_pc_dec[i] = c_pc_dec[i];
    s_pc_len[i] = c_pc_len[i];
  }

  barrier (CLK_LOCAL_MEM_FENCE);

  if (gid >= gid_max) return;

  /**
   * base
   */

  u32 word_buf[4];

  word_buf[0] = pws[gid].i[ 0];
  word_buf[1] = pws[gid].i[ 1];
  word_buf[2] = pws[gid].i[ 2];
  word_buf[3] = pws[gid].i[ 3];

  const u32 pw_len = pws[gid].pw_len;

  u32 digest[5];

  digest[0] = tmps[gid].digest_buf[0];
  digest[1] = tmps[gid].digest_buf[1];
  digest[2] = tmps[gid].digest_buf[2];
  digest[3] = tmps[gid].digest_buf[3];
  digest[4] = tmps[gid].digest_buf[4];

  /**
   * salt
   */

  u32 salt_len = salt_bufs[salt_pos].salt_len;

  u32 salt_buf[5];

  salt_buf[0] = salt_bufs[salt_pos].salt_buf[0];
  salt_buf[1] = salt_bufs[salt_pos].salt_buf[1];
  salt_buf[2] = salt_bufs[salt_pos].salt_buf[2];
  salt_buf[3] = salt_bufs[salt_pos].salt_buf[3];
  salt_buf[4] = salt_bufs[salt_pos].salt_buf[4];

  /**
   * loop
   */

  for (u32 i = 0, j = loop_pos + 1; i < loop_cnt; i++, j++)
  {
    const u32 pc_dec = s_pc_dec[j];
    const u32 pc_len = s_pc_len[j];

    u32 data0[4] = { 0, 0, 0, 0 };
    u32 data1[4] = { 0, 0, 0, 0 };
    u32 data2[4] = { 0, 0, 0, 0 };

    data0[0] = pc_dec;

    append_word (data0, data1, word_buf, pc_len);

    append_salt (data0, data1, data2, salt_buf, pc_len + pw_len);

    u32 w0[4];
    u32 w1[4];
    u32 w2[4];
    u32 w3[4];

    w0[0] = digest[0];
    w0[1] = digest[1];
    w0[2] = digest[2];
    w0[3] = digest[3];
    w1[0] = digest[4];
    w1[1] = swap32 (data0[0]);
    w1[2] = swap32 (data0[1]);
    w1[3] = swap32 (data0[2]);
    w2[0] = swap32 (data0[3]);
    w2[1] = swap32 (data1[0]);
    w2[2] = swap32 (data1[1]);
    w2[3] = swap32 (data1[2]);
    w3[0] = swap32 (data1[3]);
    w3[1] = swap32 (data2[0]);
    w3[2] = 0;
    w3[3] = (20 + pc_len + pw_len + salt_len) * 8;

    digest[0] = SHA1M_A;
    digest[1] = SHA1M_B;
    digest[2] = SHA1M_C;
    digest[3] = SHA1M_D;
    digest[4] = SHA1M_E;

    sha1_transform (w0, w1, w2, w3, digest);
  }

  tmps[gid].digest_buf[0] = digest[0];
  tmps[gid].digest_buf[1] = digest[1];
  tmps[gid].digest_buf[2] = digest[2];
  tmps[gid].digest_buf[3] = digest[3];
  tmps[gid].digest_buf[4] = digest[4];
}

__kernel void m05800_comp (__global pw_t *pws, __global const kernel_rule_t *rules_buf, __global const comb_t *combs_buf, __global const bf_t *bfs_buf, __global androidpin_tmp_t *tmps, __global void *hooks, __global const u32 *bitmaps_buf_s1_a, __global const u32 *bitmaps_buf_s1_b, __global const u32 *bitmaps_buf_s1_c, __global const u32 *bitmaps_buf_s1_d, __global const u32 *bitmaps_buf_s2_a, __global const u32 *bitmaps_buf_s2_b, __global const u32 *bitmaps_buf_s2_c, __global const u32 *bitmaps_buf_s2_d, __global plain_t *plains_buf, __global const digest_t *digests_buf, __global u32 *hashes_shown, __global const salt_t *salt_bufs, __global const void *esalt_bufs, __global u32 *d_return_buf, __global u32 *d_scryptV0_buf, __global u32 *d_scryptV1_buf, __global u32 *d_scryptV2_buf, __global u32 *d_scryptV3_buf, const u32 bitmap_mask, const u32 bitmap_shift1, const u32 bitmap_shift2, const u32 salt_pos, const u32 loop_pos, const u32 loop_cnt, const u32 il_cnt, const u32 digests_cnt, const u32 digests_offset, const u32 combs_mode, const u32 gid_max)
{
  /**
   * modifier
   */

  const u32 gid = get_global_id (0);

  if (gid >= gid_max) return;

  const u32 lid = get_local_id (0);

  /**
   * digest
   */

  const u32 r0 = tmps[gid].digest_buf[DGST_R0];
  const u32 r1 = tmps[gid].digest_buf[DGST_R1];
  const u32 r2 = tmps[gid].digest_buf[DGST_R2];
  const u32 r3 = tmps[gid].digest_buf[DGST_R3];

  #define il_pos 0

  #include COMPARE_M
}
