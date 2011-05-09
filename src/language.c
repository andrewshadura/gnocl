/**
\brief	Exploratory code, getting and setting language properities for text widget.
**/

#include "gnocl.h"
#include <string.h>
#include <assert.h>

int getLanguage ( gchar *str )
{

	/*
	http://developer.gnome.org/pango/stable/pango-Scripts-and-Languages.html#PangoLanguage

	PANGO_SCRIPT_INVALID_CODE
		a value never returned from pango_script_for_unichar()

	PANGO_SCRIPT_COMMON
		a character used by multiple different scripts

	PANGO_SCRIPT_INHERITED
		a mark glyph that takes its script from the base glyph to which it is attached
	*/

	const char *scripts[] =
	{
		"Invalid", "Common", "Inherited",

		"Arabic", "Armenian", "Bengali", "Bopomofo",
		"Cherokee", "Coptic", "	Cyrillic", "Deseret",
		"Devanagari", "Ethiopic", "Georgian", "Gothic",
		"Greek", "Gujarati", "Gurmukhi", "Han",
		"Hangul", "Hebrew", "Hiragana", "Kannada",
		"Katakana", "Khmer", "Lao", "Latin",
		"Malayalam", "Mongolian", "Myanmar", "Ogham",
		"Old Italic", "Oriya", "Runic", "Sinhala",
		"Syriac", "Tamil", "Telugu", "Thaana",
		"Thai", "Tibetan", "Canadian Aboriginal",
		"Yi", "Tagalog", "Hanunoo", "Buhid",
		"Tagbanwa", "Braille", "Cypriot",
		"Limbu", "Osmanya", "Shavian", "Linear B",
		"Tai Le", "Ugaritic", "New Tai Lue", "Buginese",
		"Glagolitic", "Tifinagh", "Syloti-Nagri", "Old Persian",
		"Kharoshthi", "unknown", "Balinese", "Cuneiform",
		"Phoenician", "Phags-pa", "N'Ko", "Kayah Li",
		"Lepcha", "Rejang", "Sundanese", "Saurashtra",
		"Cham", "Ol Chiki", "Vai", "Carian",
		"Lycian", "Lydian",
		NULL
	};

	enum scriptIdx
	{
		InvalidIdx, CommonIdx, InheritedIdx,

		ArabicIdx, ArmenianIdx, BengaliIdx, BopomofoIdx,
		CherokeeIdx, CopticIdx, CyrillicIdx, DeseretIdx,
		DevanagariIdx, EthiopicIdx, GeorgianIdx, GothicIdx,
		GreekIdx, GujaratiIdx, GurmukhiIdx, HanIdx,
		HangulIdx, HebrewIdx, HiraganaIdx, KannadaIdx,
		KatakanaIdx, KhmerIdx, LaoIdx, LatinIdx,
		MalayalamIdx, MongolianIdx, MyanmarIdx, OghamIdx,
		OldItalicIdx, OriyaIdx, RunicIdx, SinhalaIdx,
		SyriacIdx, TamilIdx, TeluguIdx, ThaanaIdx,
		ThaiIdx, TibetanIdx, CanadianAboriginalIdx,
		YiIdx, TagalogIdx, HanunooIdx, BuhidIdx,
		TagbanwaIdx, BrailleIdx, CypriotIdx,
		LimbuIdx, OsmanyaIdx, ShavianIdx, LinearBIdx,
		TaiLeIdx, UgariticIdx, NewTaiLueIdx, BugineseIdx,
		GlagoliticIdx, TifinaghIdx, SylotiNagriIdx, OldPersianIdx,
		KharoshthiIdx, UnknownIdx, BalineseIdx, CuneiformIdx,
		PhoenicianIdx, PhagspaIdx, NKoIdx, KayahLiIdx,
		LepchaIdx, RejangIdx, SundaneseIdx, SaurashtraIdx,
		ChamIdx, OlChikiIdx, VaiIdx, CarianIdx,
		LycianIdx, LydianIdx
	};

	gint idx;
	gint scr;

	getIdx ( scripts, str, &idx );


	switch ( idx )
	{

		case ArabicIdx:
			{
				scr = PANGO_SCRIPT_ARABIC;
			}
			break;
		case ArmenianIdx:
			{
				scr = PANGO_SCRIPT_ARMENIAN;
			}
			break;
		case BengaliIdx:
			{
				scr = PANGO_SCRIPT_BENGALI;
			}
			break;
		case BopomofoIdx:
			{
				scr = PANGO_SCRIPT_BOPOMOFO;
			}
			break;
		case CherokeeIdx:
			{
				scr = PANGO_SCRIPT_CHEROKEE;
			}
			break;
		case CopticIdx:
			{
				scr = PANGO_SCRIPT_COPTIC;
			}
			break;
		case 	CyrillicIdx:
			{
				scr = PANGO_SCRIPT_CYRILLIC;
			}
			break;
		case DeseretIdx:
			{
				scr = PANGO_SCRIPT_DESERET;
			}
			break;
		case DevanagariIdx:
			{
				scr = PANGO_SCRIPT_DEVANAGARI;
			}
			break;
		case EthiopicIdx:
			{
				scr = PANGO_SCRIPT_ETHIOPIC;
			}
			break;
		case GeorgianIdx:
			{
				scr = PANGO_SCRIPT_GEORGIAN;
			}
			break;
		case GothicIdx:
			{
				scr = PANGO_SCRIPT_GOTHIC;
			}
			break;
		case GreekIdx:
			{
				scr = PANGO_SCRIPT_GREEK;
			}
			break;
		case GujaratiIdx:
			{
				scr = PANGO_SCRIPT_GUJARATI;
			}
			break;
		case GurmukhiIdx:
			{
				scr = PANGO_SCRIPT_GURMUKHI;
			}
			break;
		case HanIdx:
			{
				scr = PANGO_SCRIPT_HAN;
			}
			break;
		case HangulIdx:
			{
				scr = PANGO_SCRIPT_HANGUL;
			}
			break;
		case HebrewIdx:
			{
				scr = PANGO_SCRIPT_HEBREW;
			}
			break;
		case HiraganaIdx:
			{
				scr = PANGO_SCRIPT_HIRAGANA;
			}
			break;
		case KannadaIdx:
			{
				scr = PANGO_SCRIPT_KANNADA;
			}
			break;
		case KatakanaIdx:
			{
				scr = PANGO_SCRIPT_KATAKANA;
			}
			break;
		case KhmerIdx:
			{
				scr = PANGO_SCRIPT_KHMER;
			}
			break;
		case LaoIdx:
			{
				scr = PANGO_SCRIPT_LAO;
			}
			break;
		case LatinIdx:
			{
				scr = PANGO_SCRIPT_LATIN;
			}
			break;
		case MalayalamIdx:
			{
				scr = PANGO_SCRIPT_MALAYALAM;
			}
			break;
		case MongolianIdx:
			{
				scr = PANGO_SCRIPT_MONGOLIAN;
			}
			break;
		case MyanmarIdx:
			{
				scr = PANGO_SCRIPT_MYANMAR;
			}
			break;
		case OghamIdx:
			{
				scr = PANGO_SCRIPT_OGHAM;
			}
			break;
		case OldItalicIdx:
			{
				scr = PANGO_SCRIPT_OLD_ITALIC;
			}
			break;
		case OriyaIdx:
			{
				scr = PANGO_SCRIPT_ORIYA;
			}
			break;
		case RunicIdx:
			{
				scr = PANGO_SCRIPT_RUNIC;
			}
			break;
		case SinhalaIdx:
			{
				scr = PANGO_SCRIPT_SINHALA;
			}
			break;
		case SyriacIdx:
			{
				scr = PANGO_SCRIPT_SYRIAC;
			}
			break;
		case TamilIdx:
			{
				scr = PANGO_SCRIPT_TAMIL;
			}
			break;
		case TeluguIdx:
			{
				scr = PANGO_SCRIPT_TELUGU;
			}
			break;
		case ThaanaIdx:
			{
				scr = PANGO_SCRIPT_THAANA;
			}
			break;
		case ThaiIdx:
			{
				scr = PANGO_SCRIPT_THAI;
			}
			break;
		case TibetanIdx:
			{
				scr = PANGO_SCRIPT_TIBETAN;
			}
			break;
		case CanadianAboriginalIdx:
			{
				scr = PANGO_SCRIPT_CANADIAN_ABORIGINAL;
			}
			break;
		case YiIdx:
			{
				scr = PANGO_SCRIPT_YI;
			}
			break;
		case TagalogIdx:
			{
				scr = PANGO_SCRIPT_TAGALOG;
			}
			break;
		case HanunooIdx:
			{
				scr = PANGO_SCRIPT_HANUNOO;
			}
			break;
		case BuhidIdx:
			{
				scr = PANGO_SCRIPT_BUHID;
			}
			break;
		case TagbanwaIdx:
			{
				scr = PANGO_SCRIPT_TAGBANWA;
			}
			break;
		case BrailleIdx:
			{
				scr = PANGO_SCRIPT_BRAILLE;
			}
			break;
		case CypriotIdx:
			{
				scr = PANGO_SCRIPT_CYPRIOT;
			}
			break;
		case LimbuIdx:
			{
				scr = PANGO_SCRIPT_LIMBU;
			}
			break;
		case OsmanyaIdx:
			{
				scr = PANGO_SCRIPT_OSMANYA;
			}
			break;
		case ShavianIdx:
			{
				scr = PANGO_SCRIPT_SHAVIAN;
			}
			break;
		case LinearBIdx:
			{
				scr = PANGO_SCRIPT_LINEAR_B;
			}
			break;
		case TaiLeIdx:
			{
				scr = PANGO_SCRIPT_TAI_LE;
			}
			break;
		case UgariticIdx:
			{
				scr = PANGO_SCRIPT_UGARITIC;
			}
			break;
		case NewTaiLueIdx:
			{
				scr = PANGO_SCRIPT_NEW_TAI_LUE;
			}
			break;
		case BugineseIdx:
			{
				scr = PANGO_SCRIPT_BUGINESE;
			}
			break;
		case GlagoliticIdx:
			{
				scr = PANGO_SCRIPT_GLAGOLITIC;
			}
			break;
		case TifinaghIdx:
			{
				scr = PANGO_SCRIPT_TIFINAGH;
			}
			break;
		case SylotiNagriIdx:
			{
				scr = PANGO_SCRIPT_SYLOTI_NAGRI;
			}
			break;
		case OldPersianIdx:
			{
				scr = PANGO_SCRIPT_OLD_PERSIAN;
			}
			break;
		case KharoshthiIdx:
			{
				scr = PANGO_SCRIPT_KHAROSHTHI;
			}
			break;
		case UnknownIdx:
			{
				scr = PANGO_SCRIPT_UNKNOWN;
			}
			break;
		case BalineseIdx:
			{
				scr = PANGO_SCRIPT_BALINESE;
			}
			break;
		case CuneiformIdx:
			{
				scr = PANGO_SCRIPT_CUNEIFORM;
			}
			break;
		case PhoenicianIdx:
			{
				scr = PANGO_SCRIPT_PHOENICIAN;
			}
			break;
		case PhagspaIdx:
			{
				PANGO_SCRIPT_PHAGS_PA;
			}
			break;
		case NKoIdx:
			{
				PANGO_SCRIPT_NKO;
			}
			break;
		case KayahLiIdx:
			{
				PANGO_SCRIPT_KAYAH_LI;
			}
			break;
		case LepchaIdx:
			{
				PANGO_SCRIPT_LEPCHA;
			}
			break;
		case RejangIdx:
			{
				PANGO_SCRIPT_REJANG;
			}
			break;
		case SundaneseIdx:
			{
				PANGO_SCRIPT_SUNDANESE;
			}
			break;
		case SaurashtraIdx:
			{
				PANGO_SCRIPT_SAURASHTRA;
			}
			break;
		case ChamIdx:
			{
				PANGO_SCRIPT_CHAM;
			}
			break;
		case OlChikiIdx:
			{
				PANGO_SCRIPT_OL_CHIKI;
			}
			break;
		case VaiIdx:
			{
				PANGO_SCRIPT_VAI;
			}
			break;
		case CarianIdx:
			{
				scr = PANGO_SCRIPT_CARIAN;
			}
			break;
		case LycianIdx:
			{
				scr = PANGO_SCRIPT_LYCIAN;
			}
			break;
		case LydianIdx:
			{
				scr = PANGO_SCRIPT_LYDIAN;
			}
			break;
		default:
			{
			}
	}


	g_print ( "%d----->%s\n", scr, scripts[scr] );

	return scr;
}
