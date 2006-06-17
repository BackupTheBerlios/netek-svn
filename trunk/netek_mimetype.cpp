#include "netek_mimetype.h"

static const char str_1[] = "application/andrew-inset";
static const char str_2[] = "application/atom+xml";
static const char str_3[] = "application/mac-binhex40";
static const char str_4[] = "application/mac-compactpro";
static const char str_5[] = "application/mathml+xml";
static const char str_6[] = "application/msword";
static const char str_7[] = "application/octet-stream";
static const char str_8[] = "application/oda";
static const char str_9[] = "application/ogg";
static const char str_10[] = "application/pdf";
static const char str_11[] = "application/postscript";
static const char str_12[] = "application/rdf+xml";
static const char str_13[] = "application/smil";
static const char str_14[] = "application/srgs";
static const char str_15[] = "application/srgs+xml";
static const char str_16[] = "application/vnd.mif";
static const char str_17[] = "application/vnd.mozilla.xul+xml";
static const char str_18[] = "application/vnd.ms-excel";
static const char str_19[] = "application/vnd.ms-powerpoint";
static const char str_65[] = "application/vnd.rn-realmedia";
static const char str_20[] = "application/vnd.wap.wbxml";
static const char str_21[] = "application/vnd.wap.wmlc";
static const char str_22[] = "application/vnd.wap.wmlscriptc";
static const char str_23[] = "application/voicexml+xml";
static const char str_24[] = "application/x-bcpio";
static const char str_25[] = "application/x-cdlink";
static const char str_26[] = "application/x-chess-pgn";
static const char str_27[] = "application/x-cpio";
static const char str_28[] = "application/x-csh";
static const char str_29[] = "application/x-director";
static const char str_30[] = "application/x-dvi";
static const char str_31[] = "application/x-futuresplash";
static const char str_32[] = "application/x-gtar";
static const char str_33[] = "application/x-hdf";
static const char str_34[] = "application/x-javascript";
static const char str_35[] = "application/x-koan";
static const char str_36[] = "application/x-latex";
static const char str_117[] = "application/x-ms-wmd";
static const char str_116[] = "application/x-ms-wmz";
static const char str_37[] = "application/x-netcdf";
static const char str_38[] = "application/x-sh";
static const char str_39[] = "application/x-shar";
static const char str_40[] = "application/x-shockwave-flash";
static const char str_41[] = "application/x-stuffit";
static const char str_42[] = "application/x-sv4cpio";
static const char str_43[] = "application/x-sv4crc";
static const char str_44[] = "application/x-tar";
static const char str_45[] = "application/x-tcl";
static const char str_46[] = "application/x-tex";
static const char str_47[] = "application/x-texinfo";
static const char str_48[] = "application/x-troff";
static const char str_49[] = "application/x-troff-man";
static const char str_50[] = "application/x-troff-me";
static const char str_51[] = "application/x-troff-ms";
static const char str_52[] = "application/x-ustar";
static const char str_53[] = "application/x-wais-source";
static const char str_54[] = "application/xhtml+xml";
static const char str_56[] = "application/xml";
static const char str_57[] = "application/xml-dtd";
static const char str_55[] = "application/xslt+xml";
static const char str_58[] = "application/zip";
static const char str_59[] = "audio/basic";
static const char str_60[] = "audio/midi";
static const char str_61[] = "audio/mpeg";
static const char str_62[] = "audio/x-aiff";
static const char str_63[] = "audio/x-mpegurl";
static const char str_111[] = "audio/x-ms-wax";
static const char str_110[] = "audio/x-ms-wma";
static const char str_112[] = "audio/x-ms-wmv";
static const char str_64[] = "audio/x-pn-realaudio";
static const char str_66[] = "audio/x-wav";
static const char str_67[] = "chemical/x-pdb";
static const char str_68[] = "chemical/x-xyz";
static const char str_69[] = "image/bmp";
static const char str_70[] = "image/cgm";
static const char str_71[] = "image/gif";
static const char str_72[] = "image/ief";
static const char str_73[] = "image/jpeg";
static const char str_74[] = "image/png";
static const char str_75[] = "image/svg+xml";
static const char str_76[] = "image/tiff";
static const char str_77[] = "image/vnd.djvu";
static const char str_78[] = "image/vnd.wap.wbmp";
static const char str_79[] = "image/x-cmu-raster";
static const char str_80[] = "image/x-icon";
static const char str_81[] = "image/x-portable-anymap";
static const char str_82[] = "image/x-portable-bitmap";
static const char str_83[] = "image/x-portable-graymap";
static const char str_84[] = "image/x-portable-pixmap";
static const char str_85[] = "image/x-rgb";
static const char str_86[] = "image/x-xbitmap";
static const char str_87[] = "image/x-xpixmap";
static const char str_88[] = "image/x-xwindowdump";
static const char str_89[] = "model/iges";
static const char str_90[] = "model/mesh";
static const char str_91[] = "model/vrml";
static const char str_92[] = "text/calendar";
static const char str_93[] = "text/css";
static const char str_94[] = "text/html";
static const char str_95[] = "text/plain";
static const char str_96[] = "text/richtext";
static const char str_97[] = "text/rtf";
static const char str_98[] = "text/sgml";
static const char str_99[] = "text/tab-separated-values";
static const char str_100[] = "text/vnd.wap.wml";
static const char str_101[] = "text/vnd.wap.wmlscript";
static const char str_102[] = "text/x-setext";
static const char str_103[] = "video/mpeg";
static const char str_104[] = "video/quicktime";
static const char str_105[] = "video/vnd.mpegurl";
static const char str_109[] = "video/x-ms-asf";
static const char str_114[] = "video/x-ms-wm";
static const char str_115[] = "video/x-ms-wmx";
static const char str_113[] = "video/x-ms-wvx";
static const char str_106[] = "video/x-msvideo";
static const char str_107[] = "video/x-sgi-movie";
static const char str_108[] = "x-conference/x-cooltalk";

const char *neteK::mimeTypeSearch(const char *ch)
{
switch(*ch) { case '2': 
++ch; switch(*ch) { case 'p': 
++ch; switch(*ch) { case 'm': 
++ch; switch(*ch) { case '.': return str_61; default: return 0; } default: return 0; } default: return 0; } case '3': 
++ch; switch(*ch) { case 'p': 
++ch; switch(*ch) { case 'm': 
++ch; switch(*ch) { case '.': return str_61; default: return 0; } default: return 0; } default: return 0; } case 'a': 
++ch; switch(*ch) { case 'd': 
++ch; switch(*ch) { case 'o': 
++ch; switch(*ch) { case '.': return str_8; default: return 0; } default: return 0; } case 'g': 
++ch; switch(*ch) { case 'p': 
++ch; switch(*ch) { case 'm': 
++ch; switch(*ch) { case '.': return str_61; default: return 0; } default: return 0; } default: return 0; } case 'h': 
++ch; switch(*ch) { case 'l': 
++ch; switch(*ch) { case '.': return str_7; default: return 0; } default: return 0; } case 'm': 
++ch; switch(*ch) { case 'w': 
++ch; switch(*ch) { case '.': return str_110; default: return 0; } default: return 0; } case 'r': 
++ch; switch(*ch) { case '.': return str_64; default: return 0; } default: return 0; } case 'b': 
++ch; switch(*ch) { case 'd': 
++ch; switch(*ch) { case 'p': 
++ch; switch(*ch) { case '.': return str_67; default: return 0; } default: return 0; } case 'f': 
++ch; switch(*ch) { case 'i': 
++ch; switch(*ch) { case '.': return str_92; default: return 0; } default: return 0; } case 'g': 
++ch; switch(*ch) { case 'r': 
++ch; switch(*ch) { case '.': return str_85; default: return 0; } default: return 0; } default: return 0; } case 'c': 
++ch; switch(*ch) { case 'f': 
++ch; switch(*ch) { case 'i': 
++ch; switch(*ch) { case 'a': 
++ch; switch(*ch) { case '.': return str_62; default: return 0; } default: return 0; } default: return 0; } case 'l': 
++ch; switch(*ch) { case 'm': 
++ch; switch(*ch) { case 'w': 
++ch; switch(*ch) { case '.': return str_21; default: return 0; } default: return 0; } default: return 0; } case 'n': 
++ch; switch(*ch) { case '.': return str_37; default: return 0; } case 'o': 
++ch; switch(*ch) { case 'd': 
++ch; switch(*ch) { case '.': return str_6; default: return 0; } default: return 0; } case 'r': 
++ch; switch(*ch) { case 'c': 
++ch; switch(*ch) { case '4': 
++ch; switch(*ch) { case 'v': 
++ch; switch(*ch) { case 's': 
++ch; switch(*ch) { case '.': return str_43; default: return 0; } default: return 0; } default: return 0; } default: return 0; } case 's': 
++ch; switch(*ch) { case '.': return str_53; default: return 0; } default: return 0; } case 's': 
++ch; switch(*ch) { case 'a': 
++ch; switch(*ch) { case '.': return str_95; default: return 0; } case 'l': 
++ch; switch(*ch) { case 'm': 
++ch; switch(*ch) { case 'w': 
++ch; switch(*ch) { case '.': return str_22; default: return 0; } default: return 0; } default: return 0; } default: return 0; } default: return 0; } case 'd': 
++ch; switch(*ch) { case 'c': 
++ch; switch(*ch) { case 'v': 
++ch; switch(*ch) { case '.': return str_25; default: return 0; } default: return 0; } case 'i': 
++ch; switch(*ch) { case 'm': 
++ch; switch(*ch) { case '.': return str_60; default: return 0; } default: return 0; } case 'k': 
++ch; switch(*ch) { case 's': 
++ch; switch(*ch) { case '.': return str_35; default: return 0; } default: return 0; } case 'm': 
++ch; switch(*ch) { case 'w': 
++ch; switch(*ch) { case '.': return str_117; default: return 0; } default: return 0; } case 'n': 
++ch; switch(*ch) { case 's': 
++ch; switch(*ch) { case '.': return str_59; default: return 0; } default: return 0; } case 't': 
++ch; switch(*ch) { case 'd': 
++ch; switch(*ch) { case '.': return str_57; default: return 0; } default: return 0; } case 'w': 
++ch; switch(*ch) { case 'x': 
++ch; switch(*ch) { case '.': return str_88; default: return 0; } default: return 0; } default: return 0; } case 'e': 
++ch; switch(*ch) { case 'c': 
++ch; switch(*ch) { case 'i': 
++ch; switch(*ch) { case '.': return str_108; default: return 0; } default: return 0; } case 'i': 
++ch; switch(*ch) { case 'v': 
++ch; switch(*ch) { case 'o': 
++ch; switch(*ch) { case 'm': 
++ch; switch(*ch) { case '.': return str_107; default: return 0; } default: return 0; } default: return 0; } default: return 0; } case 'm': 
++ch; switch(*ch) { case '.': return str_50; default: return 0; } case 'p': 
++ch; switch(*ch) { case 'j': 
++ch; switch(*ch) { case '.': return str_73; default: return 0; } case 'm': 
++ch; switch(*ch) { case '.': return str_103; default: return 0; } default: return 0; } case 'x': 
++ch; switch(*ch) { case 'e': 
++ch; switch(*ch) { case '.': return str_7; default: return 0; } default: return 0; } default: return 0; } case 'f': 
++ch; switch(*ch) { case 'd': 
++ch; switch(*ch) { case 'c': 
++ch; switch(*ch) { case '.': return str_37; default: return 0; } case 'h': 
++ch; switch(*ch) { case '.': return str_33; default: return 0; } case 'p': 
++ch; switch(*ch) { case '.': return str_10; default: return 0; } case 'r': 
++ch; switch(*ch) { case '.': return str_12; default: return 0; } default: return 0; } case 'e': 
++ch; switch(*ch) { case 'i': 
++ch; switch(*ch) { case '.': return str_72; default: return 0; } default: return 0; } case 'f': 
++ch; switch(*ch) { case 'i': 
++ch; switch(*ch) { case 'a': 
++ch; switch(*ch) { case '.': return str_62; default: return 0; } case 't': 
++ch; switch(*ch) { case '.': return str_76; default: return 0; } default: return 0; } case 'o': 
++ch; switch(*ch) { case 'r': 
++ch; switch(*ch) { case '.': return str_48; default: return 0; } default: return 0; } default: return 0; } case 'i': 
++ch; switch(*ch) { case 'a': 
++ch; switch(*ch) { case '.': return str_62; default: return 0; } case 'g': 
++ch; switch(*ch) { case '.': return str_71; default: return 0; } case 'm': 
++ch; switch(*ch) { case '.': return str_16; default: return 0; } case 't': 
++ch; switch(*ch) { case '.': return str_76; default: return 0; } default: return 0; } case 's': 
++ch; switch(*ch) { case 'a': 
++ch; switch(*ch) { case '.': return str_109; default: return 0; } default: return 0; } case 't': 
++ch; switch(*ch) { case 'r': 
++ch; switch(*ch) { case '.': return str_97; default: return 0; } default: return 0; } case 'w': 
++ch; switch(*ch) { case 's': 
++ch; switch(*ch) { case '.': return str_40; default: return 0; } default: return 0; } default: return 0; } case 'g': 
++ch; switch(*ch) { case 'e': 
++ch; switch(*ch) { case 'p': 
++ch; switch(*ch) { case 'j': 
++ch; switch(*ch) { case '.': return str_73; default: return 0; } case 'm': 
++ch; switch(*ch) { case '.': return str_103; default: return 0; } default: return 0; } default: return 0; } case 'g': 
++ch; switch(*ch) { case 'o': 
++ch; switch(*ch) { case '.': return str_9; default: return 0; } default: return 0; } case 'm': 
++ch; switch(*ch) { case 'd': 
++ch; switch(*ch) { case '.': return str_7; default: return 0; } default: return 0; } case 'n': 
++ch; switch(*ch) { case 'p': 
++ch; switch(*ch) { case '.': return str_74; default: return 0; } default: return 0; } case 'p': 
++ch; switch(*ch) { case 'j': 
++ch; switch(*ch) { case '.': return str_73; default: return 0; } case 'm': 
++ch; switch(*ch) { case '.': return str_103; default: return 0; } default: return 0; } case 'v': 
++ch; switch(*ch) { case 's': 
++ch; switch(*ch) { case '.': return str_75; default: return 0; } default: return 0; } default: return 0; } case 'h': 
++ch; switch(*ch) { case 's': 
++ch; switch(*ch) { case '.': return str_38; case 'c': 
++ch; switch(*ch) { case '.': return str_28; default: return 0; } case 'e': 
++ch; switch(*ch) { case 'm': 
++ch; switch(*ch) { case '.': return str_90; default: return 0; } default: return 0; } case 'm': 
++ch; switch(*ch) { case '.': return str_90; default: return 0; } default: return 0; } case 'z': 
++ch; switch(*ch) { case 'l': 
++ch; switch(*ch) { case '.': return str_7; default: return 0; } default: return 0; } default: return 0; } case 'i': 
++ch; switch(*ch) { case 'a': 
++ch; switch(*ch) { case '.': return str_11; default: return 0; } case 'd': 
++ch; switch(*ch) { case 'i': 
++ch; switch(*ch) { case 'm': 
++ch; switch(*ch) { case '.': return str_60; default: return 0; } default: return 0; } default: return 0; } case 'm': 
++ch; switch(*ch) { case 's': 
++ch; switch(*ch) { case '.': return str_13; default: return 0; } default: return 0; } case 'v': 
++ch; switch(*ch) { case 'a': 
++ch; switch(*ch) { case '.': return str_106; default: return 0; } case 'd': 
++ch; switch(*ch) { case '.': return str_30; default: return 0; } default: return 0; } case 'x': 
++ch; switch(*ch) { case 'e': 
++ch; switch(*ch) { case 't': 
++ch; switch(*ch) { case '.': return str_47; default: return 0; } default: return 0; } default: return 0; } default: return 0; } case 'l': 
++ch; switch(*ch) { case 'c': 
++ch; switch(*ch) { case 't': 
++ch; switch(*ch) { case '.': return str_45; default: return 0; } default: return 0; } case 'i': 
++ch; switch(*ch) { case 'm': 
++ch; switch(*ch) { case 's': 
++ch; switch(*ch) { case '.': return str_13; default: return 0; } default: return 0; } default: return 0; } case 'l': 
++ch; switch(*ch) { case 'd': 
++ch; switch(*ch) { case '.': return str_7; default: return 0; } default: return 0; } case 'm': 
++ch; switch(*ch) { case 'g': 
++ch; switch(*ch) { case 's': 
++ch; switch(*ch) { case '.': return str_98; default: return 0; } default: return 0; } case 'h': 
++ch; switch(*ch) { case 't': 
++ch; switch(*ch) { case 'a': 
++ch; switch(*ch) { case 'm': 
++ch; switch(*ch) { case '.': return str_5; default: return 0; } default: return 0; } default: return 0; } default: return 0; } case 'r': 
++ch; switch(*ch) { case 'v': 
++ch; switch(*ch) { case '.': return str_91; default: return 0; } default: return 0; } case 't': 
++ch; switch(*ch) { case 'h': 
++ch; switch(*ch) { case '.': return str_94; case 'x': 
++ch; switch(*ch) { case '.': return str_54; default: return 0; } default: return 0; } default: return 0; } case 'w': 
++ch; switch(*ch) { case '.': return str_100; default: return 0; } case 'x': 
++ch; switch(*ch) { case '.': return str_56; case 'b': 
++ch; switch(*ch) { case 'w': 
++ch; switch(*ch) { case '.': return str_20; default: return 0; } default: return 0; } case 'r': 
++ch; switch(*ch) { case 'g': 
++ch; switch(*ch) { case '.': return str_15; default: return 0; } default: return 0; } case 'v': 
++ch; switch(*ch) { case '.': return str_23; default: return 0; } default: return 0; } default: return 0; } case 'p': 
++ch; switch(*ch) { case 's': 
++ch; switch(*ch) { case '.': return str_31; default: return 0; } default: return 0; } case 'r': 
++ch; switch(*ch) { case 'w': 
++ch; switch(*ch) { case '.': return str_91; default: return 0; } default: return 0; } case 's': 
++ch; switch(*ch) { case 'x': 
++ch; switch(*ch) { case '.': return str_56; default: return 0; } default: return 0; } case 'u': 
++ch; switch(*ch) { case 'x': 
++ch; switch(*ch) { case '.': return str_17; default: return 0; } default: return 0; } default: return 0; } case 'm': 
++ch; switch(*ch) { case 'a': 
++ch; switch(*ch) { case 'r': 
++ch; switch(*ch) { case '.': return str_64; case 'g': 
++ch; switch(*ch) { case '.': return str_14; default: return 0; } default: return 0; } default: return 0; } case 'b': 
++ch; switch(*ch) { case 'p': 
++ch; switch(*ch) { case '.': return str_82; default: return 0; } case 'x': 
++ch; switch(*ch) { case '.': return str_86; default: return 0; } default: return 0; } case 'g': 
++ch; switch(*ch) { case 'c': 
++ch; switch(*ch) { case '.': return str_70; default: return 0; } case 'p': 
++ch; switch(*ch) { case '.': return str_83; default: return 0; } case 's': 
++ch; switch(*ch) { case '.': return str_98; default: return 0; } default: return 0; } case 'k': 
++ch; switch(*ch) { case 's': 
++ch; switch(*ch) { case '.': return str_35; default: return 0; } default: return 0; } case 'n': 
++ch; switch(*ch) { case 'p': 
++ch; switch(*ch) { case '.': return str_81; default: return 0; } default: return 0; } case 'o': 
++ch; switch(*ch) { case 't': 
++ch; switch(*ch) { case 'a': 
++ch; switch(*ch) { case '.': return str_2; default: return 0; } default: return 0; } default: return 0; } case 'p': 
++ch; switch(*ch) { case 'p': 
++ch; switch(*ch) { case '.': return str_84; default: return 0; } case 'x': 
++ch; switch(*ch) { case '.': return str_87; default: return 0; } default: return 0; } case 'r': 
++ch; switch(*ch) { case '.': return str_65; default: return 0; } case 't': 
++ch; switch(*ch) { case 'h': 
++ch; switch(*ch) { case '.': return str_94; default: return 0; } default: return 0; } case 'w': 
++ch; switch(*ch) { case '.': return str_114; default: return 0; } default: return 0; } case 'n': 
++ch; switch(*ch) { case 'a': 
++ch; switch(*ch) { case 'm': 
++ch; switch(*ch) { case '.': return str_49; default: return 0; } default: return 0; } case 'g': 
++ch; switch(*ch) { case 'p': 
++ch; switch(*ch) { case '.': return str_26; default: return 0; } default: return 0; } case 'i': 
++ch; switch(*ch) { case 'b': 
++ch; switch(*ch) { case '.': return str_7; default: return 0; } default: return 0; } default: return 0; } case 'o': 
++ch; switch(*ch) { case 'c': 
++ch; switch(*ch) { case 'i': 
++ch; switch(*ch) { case '.': return str_80; default: return 0; } default: return 0; } case 'f': 
++ch; switch(*ch) { case 'n': 
++ch; switch(*ch) { case 'i': 
++ch; switch(*ch) { case 'x': 
++ch; switch(*ch) { case 'e': 
++ch; switch(*ch) { case 't': 
++ch; switch(*ch) { case '.': return str_47; default: return 0; } default: return 0; } default: return 0; } default: return 0; } default: return 0; } default: return 0; } case 'i': 
++ch; switch(*ch) { case 'p': 
++ch; switch(*ch) { case 'c': 
++ch; switch(*ch) { case '.': return str_27; case '4': 
++ch; switch(*ch) { case 'v': 
++ch; switch(*ch) { case 's': 
++ch; switch(*ch) { case '.': return str_42; default: return 0; } default: return 0; } default: return 0; } case 'b': 
++ch; switch(*ch) { case '.': return str_24; default: return 0; } default: return 0; } default: return 0; } default: return 0; } case 'l': 
++ch; switch(*ch) { case 'i': 
++ch; switch(*ch) { case 's': 
++ch; switch(*ch) { case '.': return str_90; default: return 0; } default: return 0; } default: return 0; } case 's': 
++ch; switch(*ch) { case '.': return str_7; default: return 0; } default: return 0; } case 'p': 
++ch; switch(*ch) { case 'i': 
++ch; switch(*ch) { case 'z': 
++ch; switch(*ch) { case '.': return str_58; default: return 0; } default: return 0; } case 'k': 
++ch; switch(*ch) { case 's': 
++ch; switch(*ch) { case '.': return str_35; default: return 0; } default: return 0; } case 'm': 
++ch; switch(*ch) { case 'b': 
++ch; switch(*ch) { case '.': return str_69; case 'w': 
++ch; switch(*ch) { case '.': return str_78; default: return 0; } default: return 0; } default: return 0; } default: return 0; } case 'r': 
++ch; switch(*ch) { case 'a': 
++ch; switch(*ch) { case 'h': 
++ch; switch(*ch) { case 's': 
++ch; switch(*ch) { case '.': return str_39; default: return 0; } default: return 0; } case 'k': 
++ch; switch(*ch) { case '.': return str_60; default: return 0; } case 't': 
++ch; switch(*ch) { case '.': return str_44; case 'g': 
++ch; switch(*ch) { case '.': return str_32; default: return 0; } case 's': 
++ch; switch(*ch) { case 'u': 
++ch; switch(*ch) { case '.': return str_52; default: return 0; } default: return 0; } default: return 0; } default: return 0; } case 'c': 
++ch; switch(*ch) { case 'd': 
++ch; switch(*ch) { case '.': return str_29; default: return 0; } default: return 0; } case 'i': 
++ch; switch(*ch) { case 'd': 
++ch; switch(*ch) { case '.': return str_29; default: return 0; } default: return 0; } case 't': 
++ch; switch(*ch) { case '.': return str_48; default: return 0; } case 'x': 
++ch; switch(*ch) { case 'd': 
++ch; switch(*ch) { case '.': return str_29; default: return 0; } default: return 0; } default: return 0; } case 's': 
++ch; switch(*ch) { case 'a': 
++ch; switch(*ch) { case 'r': 
++ch; switch(*ch) { case '.': return str_79; default: return 0; } default: return 0; } case 'c': 
++ch; switch(*ch) { case 'i': 
++ch; switch(*ch) { case '.': return str_92; default: return 0; } default: return 0; } case 'e': 
++ch; switch(*ch) { case 'g': 
++ch; switch(*ch) { case 'i': 
++ch; switch(*ch) { case '.': return str_89; default: return 0; } default: return 0; } default: return 0; } case 'g': 
++ch; switch(*ch) { case 'i': 
++ch; switch(*ch) { case '.': return str_89; default: return 0; } default: return 0; } case 'j': 
++ch; switch(*ch) { case '.': return str_34; default: return 0; } case 'l': 
++ch; switch(*ch) { case 'm': 
++ch; switch(*ch) { case 'w': 
++ch; switch(*ch) { case '.': return str_101; default: return 0; } default: return 0; } case 'x': 
++ch; switch(*ch) { case '.': return str_18; default: return 0; } default: return 0; } case 'm': 
++ch; switch(*ch) { case '.': return str_51; case 'd': 
++ch; switch(*ch) { case '.': return str_7; default: return 0; } default: return 0; } case 'p': 
++ch; switch(*ch) { case '.': return str_11; case 'e': 
++ch; switch(*ch) { case '.': return str_11; default: return 0; } default: return 0; } case 's': 
++ch; switch(*ch) { case 'a': 
++ch; switch(*ch) { case 'l': 
++ch; switch(*ch) { case 'c': 
++ch; switch(*ch) { case '.': return str_7; default: return 0; } default: return 0; } default: return 0; } case 'c': 
++ch; switch(*ch) { case '.': return str_93; default: return 0; } default: return 0; } default: return 0; } case 't': 
++ch; switch(*ch) { case '.': return str_48; case 'h': 
++ch; switch(*ch) { case 'x': 
++ch; switch(*ch) { case '.': return str_54; default: return 0; } default: return 0; } case 'i': 
++ch; switch(*ch) { case 's': 
++ch; switch(*ch) { case '.': return str_41; default: return 0; } default: return 0; } case 'k': 
++ch; switch(*ch) { case 's': 
++ch; switch(*ch) { case '.': return str_35; default: return 0; } default: return 0; } case 'l': 
++ch; switch(*ch) { case 's': 
++ch; switch(*ch) { case 'x': 
++ch; switch(*ch) { case '.': return str_55; default: return 0; } default: return 0; } default: return 0; } case 'p': 
++ch; switch(*ch) { case 'c': 
++ch; switch(*ch) { case '.': return str_4; default: return 0; } case 'p': 
++ch; switch(*ch) { case '.': return str_19; default: return 0; } default: return 0; } case 'q': 
++ch; switch(*ch) { case '.': return str_104; default: return 0; } case 'x': 
++ch; switch(*ch) { case 't': 
++ch; switch(*ch) { case '.': return str_95; default: return 0; } default: return 0; } default: return 0; } case 'u': 
++ch; switch(*ch) { case '3': 
++ch; switch(*ch) { case 'm': 
++ch; switch(*ch) { case '.': return str_63; default: return 0; } default: return 0; } case '4': 
++ch; switch(*ch) { case 'm': 
++ch; switch(*ch) { case '.': return str_105; default: return 0; } default: return 0; } case 'a': 
++ch; switch(*ch) { case '.': return str_59; default: return 0; } case 'v': 
++ch; switch(*ch) { case 'j': 
++ch; switch(*ch) { case 'd': 
++ch; switch(*ch) { case '.': return str_77; default: return 0; } default: return 0; } default: return 0; } case 'x': 
++ch; switch(*ch) { case 'm': 
++ch; switch(*ch) { case '.': return str_105; default: return 0; } default: return 0; } default: return 0; } case 'v': 
++ch; switch(*ch) { case 'a': 
++ch; switch(*ch) { case 'w': 
++ch; switch(*ch) { case '.': return str_66; default: return 0; } default: return 0; } case 'j': 
++ch; switch(*ch) { case 'd': 
++ch; switch(*ch) { case '.': return str_77; default: return 0; } default: return 0; } case 'm': 
++ch; switch(*ch) { case 'w': 
++ch; switch(*ch) { case '.': return str_112; default: return 0; } default: return 0; } case 'o': 
++ch; switch(*ch) { case 'm': 
++ch; switch(*ch) { case '.': return str_104; default: return 0; } default: return 0; } case 's': 
++ch; switch(*ch) { case 't': 
++ch; switch(*ch) { case '.': return str_99; default: return 0; } default: return 0; } default: return 0; } case 'x': 
++ch; switch(*ch) { case 'a': 
++ch; switch(*ch) { case 'w': 
++ch; switch(*ch) { case '.': return str_111; default: return 0; } default: return 0; } case 'e': 
++ch; switch(*ch) { case 't': 
++ch; switch(*ch) { case '.': return str_46; case 'a': 
++ch; switch(*ch) { case 'l': 
++ch; switch(*ch) { case '.': return str_36; default: return 0; } default: return 0; } default: return 0; } default: return 0; } case 'm': 
++ch; switch(*ch) { case 'w': 
++ch; switch(*ch) { case '.': return str_115; default: return 0; } default: return 0; } case 'q': 
++ch; switch(*ch) { case 'h': 
++ch; switch(*ch) { case '.': return str_3; default: return 0; } default: return 0; } case 's': 
++ch; switch(*ch) { case 'a': 
++ch; switch(*ch) { case '.': return str_109; default: return 0; } default: return 0; } case 't': 
++ch; switch(*ch) { case 'e': 
++ch; switch(*ch) { case '.': return str_102; default: return 0; } case 'r': 
++ch; switch(*ch) { case '.': return str_96; default: return 0; } default: return 0; } case 'v': 
++ch; switch(*ch) { case 'w': 
++ch; switch(*ch) { case '.': return str_113; default: return 0; } default: return 0; } default: return 0; } case 'z': 
++ch; switch(*ch) { case 'e': 
++ch; switch(*ch) { case '.': return str_1; default: return 0; } case 'm': 
++ch; switch(*ch) { case 'w': 
++ch; switch(*ch) { case '.': return str_116; default: return 0; } default: return 0; } case 'y': 
++ch; switch(*ch) { case 'x': 
++ch; switch(*ch) { case '.': return str_68; default: return 0; } default: return 0; } default: return 0; } default: return 0; }}
