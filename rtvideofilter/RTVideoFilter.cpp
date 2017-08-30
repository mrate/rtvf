#include <iostream>
#include <fstream>
#include <allegro.h>
#include <vector>
#include <sys/times.h>
#include <dirent.h>
#include <dlfcn.h>

#include "../modulesapi/VideoFilter.h"
#include "../decodersapi/VideoDecoder.h"

#include "VideoEncoder.h"

using namespace std;

#define FILTER_BOX_WIDTH    160
#define ORIG_WIDTH          160
#define ORIG_HEIGHT         120

#define INSERT_FILTERS_COUNT    24

#define TOP_X       20
#define TOP_Y       25
#define BOTTOM_X    50
#define BOTTOM_Y    65

#define NO_SEQUENCE         -1
#define SEQUENCE_COUNT      12

#ifdef _DEBUG
#define DEBUG_OUT(x)    cout << x;
#else
#define DEBUG_OUT(x)
#endif

/// pomocny typ - vektro filtru
typedef vector<CVideoFilter*> FilterVector_t;
/// vektor filtru
FilterVector_t filters;

int video_width;    ///< sirka okna
int video_height;   ///< vyska okna
int bpp = 32;       ///< barevna hloubka

// globalni promenne
bool   show_insert;         ///< zobrazovat pridani modulu
bool   show_options;        ///< zobrazovat nastaveni
bool   end_app;             ///< ukoncit aplikaci
bool   contain_sequences;   ///< obsahuje sekvence
bool   contain_scenario;    ///< obsahuje casovy scenar
bool   scenario_running;    ///< bezi casovy scenar
int    cur_x;               ///< x-ova pozice kurzoru
int    cur_y;               ///< y-ova pozice kurzoru
int    ins_pos;             ///< vybrany filter pri vkladani
int    ins_scroll;          ///< posuvnik pri vyberu filtru

int     qualityWidth;       ///< rozliseni filtru
int     qualityHeight;

// nastaveni
struct Options_t {
	char    configName[256];
	char    outputName[256];
	char    decoder[256];
	int     outputWidth;
	int     outputHeight;
	int     width;
	int     height;
	bool    fullscreen;
	bool    help;
	bool    dump;
	bool    record_demo;
	bool    dstqual;
} options;

/// Vycet typu argumentu
enum ArgType_t {
	A_NONE,     ///< prepinac
	A_STRING,   ///< retezec
	A_INT       ///< cislo
};

/// Struktura pro parsovani argumentu
struct Args_t {
	char        arg_text[256];  ///< text parametru
	char        arg_alt[256];   ///< alternativni text
	char        help[256];      ///< napoveda parametru
	void*        pointer;       ///< ukazatel na hodnotu parametru
	ArgType_t   type;           ///< typ parametru
};

#define OPTIONS_SIZE    12

Args_t  options_config[] = {
	/* 1 */  { "--config",    "-c",   "config_filename",      &options.configName,    A_STRING },
	/* 2 */  { "--width",     "-w",   "width",                &options.width,         A_INT },
	/* 3 */  { "--height",    "-h",   "height",               &options.height,        A_INT },
	/* 4 */  { "--fullscreen", "-f",   "",                     &options.fullscreen,    A_NONE },
	/* 5 */  { "--help",      "-?",   "",                     &options.help,          A_NONE },
	/* 6 */  { "--dump",      "-df",  "",                     &options.dump,          A_NONE },
	/* 7 */  { "--owidth",    "-ow",  "output_video_width",   &options.outputWidth,   A_INT },
	/* 8 */  { "--oheight",   "-oh",  "output_video_height",  &options.outputHeight,  A_INT },
	/* 9 */  { "--output",    "-o",   "output_filename",      &options.outputName,    A_STRING },
	/* 10 */ { "--recorddemo", "-rd",  "",                     &options.record_demo,   A_NONE },
	/* 11 */ { "--decoder",   "-d",   "decoder_name,params",  &options.decoder,       A_STRING },
	/* 12 */ { "--dstqual",   "-dq",  "",                     &options.dstqual,       A_NONE }
};

// sekvence
struct SequenceEntry_t {
	int             filter;
	unsigned long   period;
	FilterConfig_t  config;
	FilterConfig_t  config_orig;
};

/// pomocny typ - vektor sekvenci
typedef vector<SequenceEntry_t> SequenceVector_t;

struct Sequence_t {
	string              name;
	SequenceVector_t    entry;
	bool active;
	long start_time;
} sequence[SEQUENCE_COUNT];

//int     active_sequence = NO_SEQUENCE;
//long    active_time;

// scenar
struct ScenarioEntry_t {
	int    sequence;
	long   time;
};

typedef vector<ScenarioEntry_t> Scenario_t;
Scenario_t  scenario;

long     scenario_time;
int      scenario_index;

// moduly
struct LibInfo_t {
	void*                lib_handle;
	create_filter_t*     create_inst;
	char                name[256];
	char                libname[256];
};

typedef vector<LibInfo_t> LibVect_t;
LibVect_t libs;

// pro efekt
BITMAP* topframe;
BITMAP* bottomframe;

// dekodeer
void* decoder_lib_handle;

// konstanty pro konzoli
#define CONS_WHITE      0
#define CONS_GREEN      1
#define CONS_RED        2
#define CONS_NECO       3

/** Nastaveni barvy konzole
*   @param c barva
*/
void set_color( int c ) {
	switch( c ) {
	case CONS_WHITE:
		cout << "\033[0m";
		break;
	case CONS_GREEN:
		cout << "\033[32m";
		break;
	case CONS_RED:
		cout << "\033[31m";
		break;
	case CONS_NECO:
		cout << "\033[33m";
		break;
	}
}

/** Nacteni modulu
*/
void load_libs() {
	cout << "Nahravam moduly:" << endl;

	dirent* ep;
	DIR* dp;
	void* lib_handle;
	char* error;

	dp = opendir( "modules/" );
	if( dp == NULL ) {
		cout << "  * Zadne moduly nenalezeny" << endl;
		return ;
	}
	while( ( ep = readdir( dp ) ) ) {
		string name( "modules/" );
		if( ep->d_type == DT_DIR ) {
			continue;
		}
		cout << " * modul ";
		cout << ep->d_name;
		cout.width( 30 - strlen( ep->d_name ) );
		cout.fill( '.' );
		cout << "";
		name.append( ep->d_name );
		lib_handle = dlopen( name.c_str(), RTLD_NOW );
		if( lib_handle == NULL ) {
			set_color( CONS_RED );
			cout << "error" << endl;
			set_color( CONS_WHITE );
			continue;
		}
		libs.push_back( LibInfo_t() );
		LibInfo_t& back = libs.back();
		back.lib_handle = lib_handle;
		strcpy( back.libname, ep->d_name );

		get_filter_name_t* getName = ( get_filter_name_t* )dlsym( lib_handle, "get_name" );
		if( ( error = dlerror() ) != NULL ) {
			set_color( CONS_RED );
			cout << "error" << endl;
			set_color( CONS_WHITE );
			cerr << error << endl;
			libs.pop_back();
			continue ;
		}
		( *getName )( back.name );

		back.create_inst = ( create_filter_t* )dlsym( lib_handle, "create_instance" );
		if( ( error = dlerror() ) != NULL ) {
			set_color( CONS_RED );
			cout << "error" << endl;
			set_color( CONS_WHITE );
			cerr << error << endl;
			libs.pop_back();
			continue ;
		}

		set_color( CONS_GREEN );
		cout << "ok" << endl;
		set_color( CONS_WHITE );
		continue;
	}
	closedir( dp );
	cout << endl;
}

/** Uvolneni nahranych modulu
*/
void unload_libs() {
	cout << "Uvolnuji moduly..." << endl;
	for( LibVect_t::const_iterator it = libs.begin(); it != libs.end(); it++ ) {
		dlclose( it->lib_handle );
	}
}

/** Nacteni dekoderu
*/
CVideoDecoder* load_decoder( char* line ) {
	char ch;
	int i = 0;
	char libName[256];
	char* error;

	sprintf( libName, "decoders/" );
	while( ( ch = line[i++] ) != ',' ) {
		libName[8 + i] = ch;
	}
	libName[8 + i] = '\0';
	strcpy( line, line + i );

	cout << "Nahravam decoder" << endl;
	cout << " * " << libName;
	cout.width( 36 - strlen( libName ) );
	cout.fill( '.' );
	cout << "";

	decoder_lib_handle = dlopen( libName, RTLD_NOW );
	if( decoder_lib_handle == NULL ) {
		return NULL;
	}
	create_decoder_t* cd = ( create_decoder_t* )dlsym( decoder_lib_handle, "create_decoder" );
	if( ( error = dlerror() ) != NULL ) {
		set_color( CONS_RED );
		cout << "error" << endl;
		set_color( CONS_WHITE );
		cerr << error << endl;
		dlclose( decoder_lib_handle );
		return NULL;
	}

	print_help_t* ph = ( print_help_t* )dlsym( decoder_lib_handle, "print_help" );
	if( ( error = dlerror() ) != NULL ) {
		set_color( CONS_RED );
		cout << "error" << endl;
		set_color( CONS_WHITE );
		cerr << error << endl;
		dlclose( decoder_lib_handle );
		return NULL;
	}

	CVideoDecoder* res = ( *cd )();
	if( res == NULL || !res->Init( line ) ) {
		( *ph )();
		dlclose( decoder_lib_handle );
		if( res ) {
			delete res;
		}
		return NULL;
	}
	set_color( CONS_GREEN );
	cout << "ok" << endl << endl;
	set_color( CONS_WHITE );

	return res;
}

void close_decoder() {
	dlclose( decoder_lib_handle );
}

/** Vytvoreni instance modulu podle nazvu
*   @param name nazev modulu
*   @return instance modulu nebo NULL, pokud modul nebyl nalezen
*/
CVideoFilter* create_instance( char* name ) {
	for( LibVect_t::const_iterator it = libs.begin(); it != libs.end(); it++ ) {
		if( strcmp( it->name, name ) == 0 ) {
			return ( *it->create_inst )();
		}
	}
	return NULL;
}

/** Parsovani argumentu aplikace
*   @param argc pocet argumentu
*   @param argv retezce argumentu
*   @return false pri chybe
*/
bool parse_args( int argc, char* argv[] ) {
	int  arg = 1;
	int*  inta;
	bool* boola;
	char* chara;
	bool options_set[OPTIONS_SIZE];

	memset( options_set, 0, OPTIONS_SIZE );
	memset( &options, 0, sizeof( Options_t ) );
	while( arg < argc ) {
		for( int i = 0; i < OPTIONS_SIZE; i++ ) {
			if( strcmp( options_config[i].arg_text, argv[arg] ) == 0 || strcmp( options_config[i].arg_alt, argv[arg] ) == 0 ) {
				if( options_set[i] ) {
					cerr << "Duplicitni parametr: " << argv[arg] << endl;
					return false;
				}
				options_set[i] = true;
				switch( options_config[i].type ) {
				case A_NONE:
					boola = ( bool* )options_config[i].pointer;
					*boola = true;
					break;
				case A_INT:
					if( arg == argc - 1 ) {
						return false;
					}
					inta = ( int* )options_config[i].pointer;
					*inta = atoi( argv[++arg] );
					break;
				case A_STRING:
					if( arg == argc - 1 ) {
						return false;
					}
					chara = ( char* )options_config[i].pointer;
					strcpy( chara, argv[++arg] );
					break;
				}
				break;
			} else if( i == OPTIONS_SIZE - 1 ) {
				return false;
			}
		}
		arg++;
	}
	return true;
}

/** Vypsani napovedy aplikace
*/
void print_help() {
	cout << "   Parametry:" << endl << "     ";
	for( int i = 0; i < OPTIONS_SIZE; i++ ) {
		if( options_config[i].type != A_NONE ) {
			cout << "[";
		}
		cout << "[" << options_config[i].arg_text << " | " << options_config[i].arg_alt;
		if( options_config[i].type != A_NONE ) {
			cout << "] " << options_config[i].help;
		}
		cout << "]" << ( i < OPTIONS_SIZE - 1 ? " " : "" );
	}
	cout << endl;
}

/** Nacteni paru nazev=hodnota z retezce
*   @param line zdrojovy retezec
*   @param name nazev
*   @param value hodnota
*/
void read_pair( char* line, char* name, double& value ) {
	int index = 0;
	int index2 = 0;
	char tmp[256];

	while( line[index] != '=' && ( size_t )index < strlen( line ) ) {
		name[index] = line[index];
		index++;
	}
	name[index++] = 0;

	while( line[index] != 0 ) {
		tmp[index2++] = line[index++];
	}
	tmp[index2] = 0;
	value = atof( tmp );
}

/** Nacteni konfigurace
*   @param f ifstream souboru
*   @return false pri chybe
*/
bool read_config( ifstream& f ) {
	SequenceEntry_t*     entry;
	FilterConfig_t config;
	char line[256], name[256];
	int index = 0;
	int line_nr = 0;
	double value;
	CVideoFilter* filter = NULL;
	while( f.good() ) {
		f.getline( line, 256 );
		line_nr++;
		if( strlen( line ) == 0 ) {
			continue;    // prazdny radek
		}
		if( line[0] == '#' ) {
			continue;    // komentar
		}
		if( line[0] == '$' ) {
			//if(filter) filter->Config(bpp, video_width, video_height, config);
			if( filter ) {
				filter->Config( bpp, qualityWidth, qualityHeight, config );
			}
			filter = NULL;
			contain_sequences = true;
		}

		if( contain_scenario ) {
			if( line[0] == '$' ) {
				memcpy( name, line + 1, strlen( line ) - 1 );
				name[strlen( line ) - 1] = '\0';
				index = atoi( name );
				( scenario.back() ).sequence = index;
			} else {
				read_pair( line, name, value );
				if( strcmp( name, "time" ) != 0 ) {
					cerr << "Neocekavany retezec " << name << ", radek " << line_nr << endl;
					return false;
				}
				scenario.push_back( ScenarioEntry_t() );
				( scenario.back() ).time = ( int )value;
			}
		} else if( contain_sequences ) {
			// nacitani sekvenci
			switch( line[0] ) {
			case '$': // zahajeni sekvence
				memcpy( name, line + 1, strlen( line ) - 1 );
				name[strlen( line ) - 1] = '\0';
				index = atoi( name );
				if( index < 0 || index >= SEQUENCE_COUNT ) {
					cerr << "Chybne cislo sekvence: " << index << " na radku " << line_nr << endl;
					return false;
				}

				DEBUG_OUT( "Nacitam sekvenci " << index << endl );
				continue;
			case '_': // nacteni parametru
				read_pair( line + 1, name, value );
				if( entry == NULL ) {
					continue;
				}
				entry->config[name] = value;

				DEBUG_OUT( "Nacteni parametru " << name << " na hodnotu " << value << endl );
				break;
			case '@': // casovy scenar
				contain_scenario = true;
				break;
			default:
				read_pair( line, name, value );
				if( strcmp( name, "filter" ) == 0 ) {
					sequence[index].entry.push_back( SequenceEntry_t() );
					entry = &sequence[index].entry.back();
					if( value < 1 || ( size_t )value > filters.size() ) {
						cerr << "Chybne cislo filtru: " << ( int )value << " na radku " << line_nr << endl;
						return false;
					}

					entry->filter = ( int )value - 1;
					DEBUG_OUT( " >> entry.filter = " << value << endl );
				} else if( strcmp( name, "time" ) == 0 ) {
					if( entry == NULL ) {
						continue;
					}
					entry->period = ( int )value;
					DEBUG_OUT( " >> entry.period = " << value << endl );
				} else if( strcmp( name, "name" ) == 0 ) {
					sequence[index].name = string( line + strlen( name ) + 1 );
				} else {
					cerr << "Neznamy parametr " << name << " = " << value << " na radku " << line_nr << endl;
					return false;
				}
				break;
			}

		} else {
			// defaultni nastaveni filtru
			if( line[0] == '_' ) {
				read_pair( line + 1, name, value );
				DEBUG_OUT( "name=" << name << ", value=" << value << endl );
				config[name] = value;
			} else {
				memcpy( name, line, strlen( line ) );
				name[strlen( line )] = '\0';
				DEBUG_OUT( "Filter: '" << name << "'" << endl );
				//if(filter) filter->Config(bpp, video_width, video_height, config);
				if( filter ) {
					filter->Config( bpp, qualityWidth, qualityHeight, config );
				}
				config.clear();
				filter = create_instance( name ); //CFilterFactory::CreateInstance(name);
				if( filter ) {
					filters.push_back( filter );
				} else {
					cerr << "Neznamy filtr '" << name << "', radek " << line_nr << endl;
					return false;
				}
				continue;
			}
		}
	}
	//if(filter) filter->Config(bpp, video_width, video_height, config);
	if( filter ) {
		filter->Config( bpp, qualityWidth, qualityHeight, config );
	}

	/*for(int i=0;i<SEQUENCE_COUNT;i++)
	{
	    //DEBUG_OUT("sequence[" << i << "].entry.size() = " << sequence[i].entry.size() << endl);
	    for(SequenceVector_t::const_iterator it=sequence[i].entry.begin();it!=sequence[i].entry.end();it++)
	    {
	        DEBUG_OUT(" - filter=" << it->filter << endl);
	        DEBUG_OUT(" - period=" << it->period << endl);
	        for(FilterConfig_t::const_iterator it2=(*it).config.begin(); it2 != (*it).config.end(); it2++)
	        {
	            DEBUG_OUT(" --- " << it2->first << " = " << it2->second << endl);
	        }
	    }
	}*/
	return true;
}

/** Ulozeni konfigurace do souboru
*/
void save_config() {
	DEBUG_OUT( "Ukladam konfiguraci" << endl );
	ofstream f;
	f.open( "save.cfg", fstream::out );

	FilterConfig_t conf;
	string name;
	char buffer[256];
	for( FilterVector_t::const_iterator it = filters.begin(); it != filters.end(); it++ ) {
		conf.clear();
		( *it )->GetInfo( name, conf );
		f.write( name.c_str(), name.size() );
		f.write( "\n", 1 );
		for( FilterConfig_t::const_iterator it2 = conf.begin(); it2 != conf.end(); it2++ ) {
			sprintf( buffer, "%s=%.2f\n", ( *it2 ).first.c_str(), ( *it2 ).second );
			f.write( buffer, strlen( buffer ) );
		}
	}
	f.close();
}

/** Vykresleni ramecku
*   @param bmp cilova bitmapa
*   @param x1 levy okraj
*   @param y1 horni okraj
*   @param x2 pravy okraj
*   @param y2 dolni okraj
*/
void render_frame( BITMAP* bmp, int x1, int y1, int x2, int y2 ) {
	rectfill( bmp, x1, y1, x2, y2, makecol( 0, 0, 0 ) );
	rect( bmp, x1, y1, x2, y2, makecol( 255, 255, 255 ) );
	masked_blit( topframe, bmp, 0, 0, x1 - TOP_X, y1 - TOP_Y, topframe->w, topframe->h );
	masked_blit( bottomframe, bmp, 0, 0, x2 - BOTTOM_X, y2 - BOTTOM_Y, topframe->w, topframe->h );
}

/** Vykresleni napovedy
*   @param bmp bitmapa, kam se bude kreslit
*/
void render_help( BITMAP* bmp ) {
	int tcolor = makecol( 255, 255, 255 );
	int bcolor = makecol( 0, 0, 0 );

	int vpos = 20, hpos = SCREEN_H - 110, y = 0;
	rectfill( bmp, vpos, hpos, 480, SCREEN_H - 20, bcolor );
	textout_ex( bmp, font, "NUMPAD 7/9    +/- 1.00", vpos + 10, hpos + 10 + ( y++ ) * 10, tcolor, bcolor );
	textout_ex( bmp, font, "NUMPAD 4/6    +/- 0.10", vpos + 10, hpos + 10 + ( y++ ) * 10, tcolor, bcolor );
	textout_ex( bmp, font, "NUMPAD 1/3    +/- 0.01", vpos + 10, hpos + 10 + ( y++ ) * 10, tcolor, bcolor );
	y++;

	rect( bmp, vpos, hpos, 480, SCREEN_H - 20, tcolor );
	masked_blit( topframe, bmp, 0, 0, vpos - TOP_X, hpos - TOP_Y, topframe->w, topframe->h );
	masked_blit( bottomframe, bmp, 0, 0, 480 - BOTTOM_X, SCREEN_H - 20 - BOTTOM_Y, topframe->w, topframe->h );

	y = 0;
	vpos = 250;
	textout_ex( bmp, font, "I      - insert filter", vpos + 10, hpos + 10 + ( y++ ) * 10, tcolor, bcolor );
	textout_ex( bmp, font, "DEL    - remove filter", vpos + 10, hpos + 10 + ( y++ ) * 10, tcolor, bcolor );
	textout_ex( bmp, font, "PGUP   - move filter left", vpos + 10, hpos + 10 + ( y++ ) * 10, tcolor, bcolor );
	textout_ex( bmp, font, "PDDOWN - move filter right", vpos + 10, hpos + 10 + ( y++ ) * 10, tcolor, bcolor );
	textout_ex( bmp, font, "S      - save configuration", vpos + 10, hpos + 10 + ( y++ ) * 10, tcolor, bcolor );

}

/** Zpracovani sekvenci
*   @param keyp pole stisknutych klaves
*/
void handle_sequences( bool* keyp ) {
	tms tm;
	long current_time = times( &tm );
	for( int i = 0; i < SEQUENCE_COUNT; i++ ) {
		//if(active_sequence != NO_SEQUENCE)
		if( sequence[i].active ) {
			//active_sequence = i;
			bool done = true;
			//unsigned long elapsed_time = (current_time - active_time)*10;
			unsigned long elapsed_time = ( current_time - sequence[i].start_time ) * 10;
			double percent;
			FilterConfig_t config;

			// projedu aktivni sekvenci a nastavim filtry
			for( SequenceVector_t::iterator it = sequence[i].entry.begin(), end = sequence[i].entry.end(); it != end; it++ ) {
				if( elapsed_time < it->period ) {
					done = false; // aspon jeden filter se nedokoncil
					percent = ( double )elapsed_time / ( double )it->period;
				} else {
					percent = 1.0f;
				}

				// vypocitam nove nastaveni
				for( FilterConfig_t::const_iterator it2 = it->config.begin(), end2 = it->config.end(); it2 != end2; it2++ ) {
					config[it2->first] = it->config_orig[it2->first] + percent * ( it2->second - it->config_orig[it2->first] );
				}

				if( filters[it->filter] != NULL ) {
					filters[it->filter]->Config( bpp, qualityWidth, qualityHeight, config );
				}
				//filters[it->filter]->Config(bpp, video_width, video_height, config);
			}

			if( done ) {
				sequence[i].active = false;
			}
			//active_sequence = NO_SEQUENCE;
		}
	}

	for( int i = KEY_F1; i <= KEY_F12; i++ ) {
		// provede se prvni stisknuta sekvence
		if( keyp[i] ) {
			int active_sequence = i - KEY_F1;

			if( sequence[active_sequence].active ) {
				sequence[active_sequence].active = false;
				break;
			}
			sequence[active_sequence].active = true;
			sequence[active_sequence].start_time = current_time;

			string tmp;
			for( SequenceVector_t::iterator it = sequence[active_sequence].entry.begin(), end = sequence[active_sequence].entry.end(); it != end; it++ ) {
				filters[it->filter]->GetInfo( tmp, it->config_orig );
			}

			scenario_running = false;
			DEBUG_OUT( "activeTime: " << active_time << endl );
			break;
		}
	}

	if( keyp[KEY_ENTER] ) {
		scenario_running = !scenario_running;
		scenario_time = current_time;
		scenario_index = 0;
	}

	if( scenario_running ) {
		if( ( size_t )scenario_index < scenario.size() ) {
			if( current_time - scenario_time >= scenario[scenario_index].time / 10 ) {
				int active_sequence = scenario[scenario_index].sequence;

				sequence[active_sequence].active = true;
				sequence[active_sequence].start_time = current_time;

				scenario_index++;
				string tmp;
				for( SequenceVector_t::iterator it = sequence[active_sequence].entry.begin(), end = sequence[active_sequence].entry.end(); it != end; it++ ) {
					filters[it->filter]->GetInfo( tmp, it->config_orig );
				}
			}
		} else { //if(active_sequence == NO_SEQUENCE)
			scenario_running = false;
			for( int i = 0; i < SEQUENCE_COUNT; i++ )
				if( sequence[i].active ) {
					scenario_running = true;
					break;
				}
		}
	}
}

/** Vykresleni nastaveni filtru
*   @param bbuffer cilova bitmapa
*/
void render_options( BITMAP* bbuffer ) {
	string name;
	FilterConfig_t info;
	int x = 20, y, height;
	int count = 0, count2 = 0;
	int bgcolor, textcolor, nbgcolor, ntextcolor;
	char buffer[256];

	for( FilterVector_t::const_iterator it = filters.begin(); it != filters.end(); it++ ) {
		if( ( *it )->isActive() ) {
			bgcolor = ( cur_x != count || show_insert ? makecol( 0, 0, 0 ) : makecol( 0, 0, 0 ) );
			textcolor = ( cur_x != count ? makecol( 255, 255, 255 ) : makecol( 255, 255, 255 ) );
		} else {
			bgcolor = ( cur_x != count ? makecol( 128, 0, 0 ) : makecol( 128, 0, 0 ) );
			textcolor = ( cur_x != count ? makecol( 255, 255, 255 ) : makecol( 255, 255, 255 ) );
		}

		info.clear();
		( *it )->GetInfo( name, info );

		height = 28 + info.size() * 10;
		if( height < bottomframe->h ) {
			height = bottomframe->h;
		}

		rect( bbuffer, x, 0, x + FILTER_BOX_WIDTH - 10, height, makecol( 255, 255, 255 ) );
		rectfill( bbuffer, x + 1, 0, x + FILTER_BOX_WIDTH - 11, height - 1, bgcolor );

		masked_blit( bottomframe, bbuffer, 0, 0, x + FILTER_BOX_WIDTH - 10 - BOTTOM_X, height - BOTTOM_Y, topframe->w, topframe->h );

		textout_ex( bbuffer, font, name.c_str(), x + 10, 10, textcolor, bgcolor );

		y = 28;
		count2 = 0;
		for( FilterConfig_t::iterator it2 = info.begin(); it2 != info.end(); it2++ ) {
			if( !show_insert && cur_x == count && cur_y == count2 ) {
				rectfill( bbuffer, x + 5, y, x + FILTER_BOX_WIDTH - 15, y + 10, makecol( 255, 255, 255 ) );
				ntextcolor = makecol( 0, 0, 0 );
				nbgcolor = makecol( 255, 255, 255 );
			} else {
				ntextcolor = textcolor;
				nbgcolor = bgcolor;
			}

			sprintf( buffer, "%s = %.2f", it2->first.c_str(), it2->second );
			textout_ex( bbuffer, font, buffer, x + 20, y + 1, ntextcolor, nbgcolor );
			y += 10;
			count2++;
		}

		x += FILTER_BOX_WIDTH;
		count++;
	}

	if( show_insert ) {
		// vykresleni tabulky pro vlozeni filtru
		int xpos = 20, ypos = SCREEN_H - ORIG_HEIGHT - 340, i = 0, bcolor, tcolor;
		render_frame( bbuffer, xpos, ypos, xpos + 330, ypos + INSERT_FILTERS_COUNT * 12 + 24 );
		//textout_ex(bbuffer, font, text.c_str(), 100, 100, makecol(255, 255, 255), makecol(0,0,0));
		if( libs.size() > INSERT_FILTERS_COUNT ) {
			int sl = ( ( float )( ( INSERT_FILTERS_COUNT - 1 ) * 12 ) * ( float )ins_scroll / ( float )( libs.size() - INSERT_FILTERS_COUNT ) );
			rectfill( bbuffer, xpos + 320, ypos + 12 + sl, xpos + 325, ypos + sl + 24, makecol( 255, 255, 255 ) );
		}
		if( ins_scroll > 0 ) {
			triangle( bbuffer, xpos + 165, ypos + 5, xpos + 160, ypos + 10, xpos + 170, ypos + 10, makecol( 255, 255, 255 ) );
		}
		if( libs.size() > ( size_t )INSERT_FILTERS_COUNT && ins_scroll < libs.size() - INSERT_FILTERS_COUNT ) {
			triangle( bbuffer, xpos + 165, ypos + INSERT_FILTERS_COUNT * 12 + 19, xpos + 160, ypos + INSERT_FILTERS_COUNT * 12 + 14, xpos + 170, ypos + INSERT_FILTERS_COUNT * 12 + 14, makecol( 255, 255, 255 ) );
		}
		for( LibVect_t::const_iterator it = libs.begin(); it != libs.end(); it++ ) {
			if( i++ < ins_scroll ) {
				continue;
			}
			string text( it->name );
			text.append( " (" );
			text.append( it->libname );
			text.append( ")" );
			ypos += 12;
			if( i - 1 == ins_pos ) {
				rectfill( bbuffer, xpos + 5, ypos, xpos + 315, ypos + 12, makecol( 255, 255, 255 ) );
				bcolor = makecol( 255, 255, 255 );
				tcolor = makecol( 0, 0, 0 );
			} else {
				bcolor = makecol( 0, 0, 0 );
				tcolor = makecol( 255, 255, 255 );
			}
			textout_ex( bbuffer, font, text.c_str(), xpos + 10, ypos + 3, tcolor, bcolor );
			if( i >= ins_scroll + INSERT_FILTERS_COUNT ) {
				break;
			}
		}
	}
}

/** Vykresleni popisu sekvenci
*   @param bbuffer cilova bitmapa
*/
void render_sequences( BITMAP* bbuffer ) {
	int tcolor = makecol( 255, 255, 255 );
	int bcolor = makecol( 0, 0, 0 );
	int vpos = 20, hpos = SCREEN_H - 170, y = 0;
	char text[256];

	render_frame( bbuffer, vpos, hpos, vpos + 450, hpos + 150 );
	for( int i = 0; i < SEQUENCE_COUNT; i++ ) {
		if( sequence[i].entry.size() == 0 ) {
			continue;
		}

		//if(i == active_sequence) tcolor = makecol(255, 0, 0);
		if( sequence[i].active ) {
			tcolor = makecol( 255, 0, 0 );
		} else {
			tcolor = makecol( 255, 255, 255 );
		}

		sprintf( text, "F%d - %s", i + 1, sequence[i].name.c_str() );
		textout_ex( bbuffer, font, text, vpos + 10, hpos + 10 + ( y++ ) * 10, tcolor, bcolor );
	}

	if( contain_scenario ) {
		// rozvrzeni scenare
		int i = 0;
		vpos = 500;
		hpos = SCREEN_H - 170;
		y = 0;
		render_frame( bbuffer, vpos, hpos, vpos + 350, hpos + 150 );
		for( Scenario_t::const_iterator it = scenario.begin(); it != scenario.end(); it++ ) {
			if( i == scenario_index - 1 && scenario_running ) {
				if( sequence[i].active ) {
					tcolor = makecol( 255, 0, 0 );
				} else {
					tcolor = makecol( 255, 255, 0 );
				}
			} else {
				tcolor = makecol( 255, 255, 255 );
			}
			long t = it->time / 10;
			textprintf_ex( bbuffer, font, vpos + 10, hpos + 10 + ( y++ ) * 10, tcolor, bcolor,
			               "%02ld:%02ld:%02ld - %s", t / 6000, ( t / 100 ) % 60, t % 100, sequence[it->sequence].name.c_str() );
			i++;
		}

		render_frame( bbuffer, SCREEN_W - 100, SCREEN_H - 100, SCREEN_W - 20, SCREEN_H - 20 );
		if( scenario_running ) {
			// cas
			triangle( bbuffer, SCREEN_W - 75, SCREEN_H - 75, SCREEN_W - 75, SCREEN_H - 45, SCREEN_W - 45, SCREEN_H - 60, makecol( 0, 255, 0 ) );
			tms tm;
			long ctime = times( &tm ) - scenario_time;
			textprintf_centre_ex( bbuffer, font, SCREEN_W - 60, SCREEN_H - 35,
			                      makecol( 0, 255, 0 ), makecol( 0, 0, 0 ), "%02ld:%02ld:%02ld", ctime / 6000, ( ctime / 100 ) % 60, ctime % 100 );
		} else {
			rectfill( bbuffer, SCREEN_W - 75, SCREEN_H - 75, SCREEN_W - 45, SCREEN_H - 45, makecol( 255, 0, 0 ) );
			textout_centre_ex( bbuffer, font, "00:00:00", SCREEN_W - 60, SCREEN_H - 35,
			                   makecol( 255, 0, 0 ), makecol( 0, 0, 0 ) );
		}
	}
}

/** Zpracovani vstupu z klavesnice
*   @param keyp pole stisknutych klaves
*/
void handle_input( bool* keyp ) {
	string name;
	bool reconfig = false;
	FilterConfig_t info;
	int count = filters.size();

	if( count > 0 && !show_insert ) {
		filters[cur_x]->GetInfo( name, info );

		FilterConfig_t::iterator it = info.begin();
		for( int i = 0; i < cur_y; i++ ) {
			it++;
		}
		if( keyp[KEY_1_PAD] || keyp[KEY_T] ) {
			info[it->first] = it->second - 0.01;
			reconfig = true;
		}
		if( keyp[KEY_3_PAD] || keyp[KEY_G] ) {
			info[it->first] = it->second + 0.01;
			reconfig = true;
		}
		if( keyp[KEY_4_PAD] || keyp[KEY_Y] ) {
			info[it->first] = it->second - 0.1;
			reconfig = true;
		}
		if( keyp[KEY_6_PAD] || keyp[KEY_H] ) {
			info[it->first] = it->second + 0.1;
			reconfig = true;
		}
		if( keyp[KEY_7_PAD] || keyp[KEY_U] ) {
			info[it->first] = it->second - 1.0;
			reconfig = true;
		}
		if( keyp[KEY_9_PAD] || keyp[KEY_J] ) {
			info[it->first] = it->second + 1.0;
			reconfig = true;
		}
		//if(reconfig) filters[cur_x]->Config(bpp, video_width, video_height, info);
		if( reconfig ) {
			filters[cur_x]->Config( bpp, qualityWidth, qualityHeight, info );
		}

		// zpracovani klaves
		if( keyp[KEY_RIGHT] ) {
			if( ++cur_x >= count ) {
				cur_x = count - 1;
			}
			cur_y = 0;
		} else if( keyp[KEY_LEFT] ) {
			if( --cur_x < 0 ) {
				cur_x = 0;
			}
			cur_y = 0;
		}
		if( keyp[KEY_PGUP] && cur_x > 0 ) {
			CVideoFilter* tmp = filters[cur_x];
			filters[cur_x] = filters[cur_x - 1];
			filters[cur_x - 1] = tmp;
			cur_x--;
		} else if( keyp[KEY_PGDN] && cur_x < count - 1 ) {
			CVideoFilter* tmp = filters[cur_x];
			filters[cur_x] = filters[cur_x + 1];
			filters[cur_x + 1] = tmp;
			cur_x++;
		} else if( keyp[KEY_DEL] && filters.size() > 0 ) {
			FilterVector_t::iterator it = filters.begin();
			for( int i = 0; ( i++ ) < cur_x; it++ ) {
				;
			}
			delete( *it );
			filters.erase( it );
			if( cur_x > 0 ) {
				cur_x--;
			}
			cur_y = 0;
		} else if( keyp[KEY_UP] ) {
			if( cur_y > 0 ) {
				cur_y--;
			}
		} else if( keyp[KEY_DOWN] ) {
			if( ( size_t )cur_y < info.size() - 1 ) {
				cur_y++;
			}
		} else if( keyp[KEY_SPACE] ) {
			filters[cur_x]->Activate( !filters[cur_x]->isActive() );
		} else if( keyp[KEY_D] ) {
			filters[cur_x]->Default();
		}
	} else if( show_insert ) {
		if( keyp[KEY_UP] ) {
			ins_pos--;
		}
		if( keyp[KEY_DOWN] ) {
			ins_pos++;
		}
		if( keyp[KEY_PGUP] ) {
			ins_pos -= INSERT_FILTERS_COUNT;
		}
		if( keyp[KEY_PGDN] ) {
			ins_pos += INSERT_FILTERS_COUNT;
		}

		if( ins_pos < 0 ) {
			ins_pos = 0;
		}
		if( ( size_t )ins_pos >= libs.size() - 1 ) {
			ins_pos = libs.size() - 1;
		}
		if( ins_pos < ins_scroll ) {
			ins_scroll = ins_pos;
		}
		if( ins_pos >= ins_scroll + INSERT_FILTERS_COUNT ) {
			ins_scroll++;
		}

		if( keyp[KEY_ENTER] ) {
			info.clear();
			filters.push_back( ( *libs[ins_pos].create_inst )() );
			//(filters.back())->Config(bpp, video_width, video_height, info);
			( filters.back() )->Config( bpp, qualityWidth, qualityHeight, info );
			show_insert = false;
			cur_x = filters.size() - 1;
			cur_y = 0;
		}
	}

	// ridici klavesy
	if( keyp[KEY_I] && !contain_sequences ) {
		show_insert = !show_insert;
	}
	if( keyp[KEY_O] ) {
		show_options = !show_options;
	}
	if( keyp[KEY_S] ) {
		save_config();
	}
	if( !show_options ) {
		show_insert = false;
	}
	if( keyp[KEY_ESC] ) {
		if( show_insert ) {
			show_insert = false;
		} else if( show_options ) {
			show_options = false;
		} else {
			end_app = true;
		}
	}
}

/// main
int main( int argc, char* argv[] ) {
	CVideoEncoder* encoder = NULL;
	CVideoDecoder* decoder = NULL;
	BITMAP*      frame;
	BITMAP*      orig;
	BITMAP*      bbuffer;
	bool        key_pressed[KEY_MAX];

	scenario_running = false;
	contain_scenario = false;
	contain_sequences = false;
	show_options = false;
	show_insert = false;
	end_app = false;
	ins_pos = 0;
	ins_scroll = 0;

	cout << "RTVideoFilter";
	cout.width( 57 );
	cout << "2008/2009 Tomas Kotal" << endl;
	cout << "--------------------------------------------------------------------------------" << endl << endl;

	// inicializace allegra
	allegro_init();
	install_keyboard();

	// zpracovani parametru
	if( !parse_args( argc, argv ) ) {
		cerr << "Chybny parametr" << endl;
		print_help();
		return -1;
	} else if( options.help ) {
		print_help();
		return 0;
	}

	if( strlen( options.decoder ) == 0 ) {
		cerr << "Chybi dekoder" << endl;
		return -1;
	}
	decoder = load_decoder( options.decoder );
	if( decoder == NULL ) {
		cerr << "Chyba inicializace dekoderu" << endl;
		return -1;
	}

	// nacteni modulu
	load_libs();

	video_width = decoder->Width();
	video_height = decoder->Height();
	if( options.dump ) {
		decoder->DumpFormat();
		if( encoder ) {
			encoder->DumpFormat();
		}
	}

	if( options.dstqual ) {
		qualityWidth = ( options.width > 0 ? options.width : video_width );
		qualityHeight = ( options.height > 0 ? options.height : video_height );
	} else {
		qualityWidth = video_width;
		qualityHeight = video_height;
	}

	// pokud byl predan konfiguracni soubor tak nacteni
	if( strlen( options.configName ) > 0 ) {
		ifstream f( options.configName );
		if( !f.good() ) {
			cerr << "Chyba otevirani konfiguracniho souboru" << endl;
			return -1;
		}
		if( !read_config( f ) ) {
			f.close();
			return -1;
		}
		f.close();
	}

	// pokud byl nastaven vystup
	if( strlen( options.outputName ) > 0 ) {
		int iw = video_width;
		int ih = video_height;
		int ow = options.outputWidth;
		int oh = options.outputHeight;
		if( ow == 0 || oh == 0 ) {
			ow = video_width;
			oh = video_height;
		}
		if( options.record_demo ) {
			iw = options.width;
			ih = options.height;
		}
		cout << "Rozliseni vystupu: " << ow << "x" << oh << endl;

		encoder = new CVideoEncoder();
		if( !encoder->Open( options.outputName, iw, ih, ow, oh ) ) {
			cout << "output error" << endl;
			return -1;
		}
	}

	// nastaveni grafickeho rezimu
	set_color_depth( bpp );
	set_gfx_mode( options.fullscreen ? GFX_AUTODETECT_FULLSCREEN : GFX_AUTODETECT_WINDOWED,
	              ( options.width > 0 ? options.width : video_width ),
	              ( options.height > 0 ? options.height : video_height ),
	              0,
	              0 );
	clear_keybuf();

	// inicializace pomocnych bitmap
	bbuffer = create_bitmap_ex( bpp, SCREEN_W, SCREEN_H );
	if( bbuffer == NULL ) {
		cerr << "Chyba pri vytvareni video bitmapy" << endl;
		return -1;
	}
	frame = create_bitmap_ex( bpp, video_width, video_height );
	orig = create_bitmap_ex( bpp, ORIG_WIDTH, ORIG_HEIGHT );

	// nacteni grafiky
	topframe = load_bitmap( "res/topframe.pcx", NULL );
	bottomframe = load_bitmap( "res/bottomframe.pcx", NULL );
	if( !topframe || !bottomframe ) {
		cerr << "Chyba nacitani grafiky" << endl;
		return -1;
	}

	// hlavni smycka
	while( decoder->GetNextFrame( frame ) && !end_app ) {
		// nacteni vstupu
		memset( key_pressed, 0, KEY_MAX );
		while( keypressed() ) {
			key_pressed[( readkey() >> 8 )] = true;
		}

		// zkopirovani originalniho snimku do nahledu
		if( show_options ) {
			stretch_blit( frame, orig, 0, 0, frame->w, frame->h, 0, 0, orig->w, orig->h );
		}

		// zpracovani sekvenci
		if( contain_sequences ) {
			handle_sequences( key_pressed );
		}

		if( options.dstqual ) {
			if( video_width == SCREEN_W && video_height == SCREEN_H ) {
				blit( frame, bbuffer, 0, 0, 0, 0, SCREEN_W, SCREEN_H );
			} else {
				stretch_blit( frame, bbuffer, 0, 0, video_width, video_height, 0, 0, SCREEN_W, SCREEN_H );
			}

			// aplikace filtru
			for( FilterVector_t::const_iterator it = filters.begin(); it != filters.end(); it++ ) {
				( *it )->Apply( bbuffer );
			}
		} else {
			for( FilterVector_t::const_iterator it = filters.begin(); it != filters.end(); it++ ) {
				( *it )->Apply( frame );
			}

			// zobrazeni upraveneho snimku
			if( video_width == SCREEN_W && video_height == SCREEN_H ) {
				blit( frame, bbuffer, 0, 0, 0, 0, SCREEN_W, SCREEN_H );
			} else {
				stretch_blit( frame, bbuffer, 0, 0, video_width, video_height, 0, 0, SCREEN_W, SCREEN_H );
			}
		}

		if( contain_sequences && show_options ) {
			// u sekvenci zobrazeni napovedy
			render_sequences( bbuffer );
			render_options( bbuffer );
		} else if( show_options ) {
			// vykresleni nahledu s nastavenim filtru
			blit( orig, bbuffer, 0, 0, SCREEN_W - orig->w - 20, SCREEN_H - orig->h - 20, SCREEN_W - 20, SCREEN_H - 20 );
			rect( bbuffer, SCREEN_W - orig->w - 21, SCREEN_H - orig->h - 21, SCREEN_W - 19, SCREEN_H - 19, makecol( 255, 255, 255 ) );
			render_options( bbuffer );
			render_help( bbuffer );
		}

		// zpracovani vstupu
		handle_input( key_pressed );

		//show_video_bitmap(bbuffer);
		blit( bbuffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H );

		if( encoder ) {
			encoder->EncodeFrame( options.record_demo ? bbuffer : frame );
		}
	}

	if( encoder ) {
		encoder->Close();
		delete encoder;
	}

	// uvolneni dekoderu
	decoder->Close();
	delete decoder;
	close_decoder();

	// uvolneni filtru
	for( FilterVector_t::const_iterator it = filters.begin(); it != filters.end(); it++ ) {
		delete *it;
	}

	unload_libs();

	// uvolneni bitmap
	/*destroy_bitmap(orig);
	destroy_bitmap(bbuffer);
	destroy_bitmap(frame);
	destroy_bitmap(topframe);
	destroy_bitmap(bottomframe);*/

	return 0;
}
