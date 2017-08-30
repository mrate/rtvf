#ifndef _VIDEOFILTER_H_
#define _VIDEOFILTER_H_

#include <allegro.h>
#include <string>
#include <map>
#include <vector>

/// Typ konfigurace filtru
typedef std::map<std::string, double> FilterConfig_t;

/// Hlavni trida video filtru
class CVideoFilter
{
protected:
    bool active; ///< filtr je aktivni

public:

    CVideoFilter() : active(true) {};
    virtual ~CVideoFilter() {};

    /** aplikace filtru na bitmapu
    *   @param bmp cilova bitmapa
    */
    virtual void Apply(BITMAP *bmp) = 0;

    /// inicializace filtru
    virtual void Init() {};

    /** nastaveni filtru
    *   @param bpp barevna hloubka
    *   @param w sirka vstupu
    *   @param h sirka vystupu
    *   @param c konfigurace filtru
    */
    virtual void Config(int bpp, int w, int h, FilterConfig_t &c) {};

    /** Zjisteni informace o filtru
    *   @param name nazev filtru
    *   @param info aktualni nastaveni filtru
    */
    virtual void GetInfo(std::string &name, FilterConfig_t &info) = 0;

    /// Nastaveni defaultnich hodnot filtru
    virtual void Default() = 0;

    /** Aktivace filtru
    *   @param a aktivace
    */
    void Activate(bool a) { active = a; };

    /** Dotaz na aktivitu filtru
    *   @return true pokud je filtr aktivni
    */
    bool isActive() { return active; };
};

// Pomocne typy pro vyslednou knihovnu

/// Vytvoreni instance filtru
typedef CVideoFilter* create_filter_t();

/// Ziskani textovyho nazvu filtru
typedef void get_filter_name_t(char *);

#endif // _VIDEOFILTER_H_
