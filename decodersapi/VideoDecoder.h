#ifndef _VIDEODECODER_H_
#define _VIDEODECODER_H_

#include <allegro.h>
#include <iostream>

class CVideoDecoder
{
public:
    /** Inicializace dekoderu
    *   @param params retezec s parametry pro dekoder, je predavan z prikazove radky
    *           pri spousteni programu
    *   @return false pri chybe
    */
    virtual bool Init(char *params) = 0;

    /// Ukonceni prace s dekoderem
    virtual void Close() = 0;

    /// Dump vstupniho formatu na stdout
    virtual void DumpFormat() = 0;

    /** Dekodovani nasledujici bitmapy
    *   @param bmp pointer na inicializovanou bitmapu, ktera bude naplnena dekodovanym snimkem
    *   @return false pri chybe
    */
    virtual bool GetNextFrame(BITMAP *bmp) = 0;

    /** Sirka dekodovaneho obrazu
    *   @return sirka
    */
    virtual int Width() = 0;

    /** Vyska dekodovaneho obrazu
    *   @return vyska
    */
    virtual int Height() = 0;
};

// pomocne typy pro dynamicke knihovny

/// Vytvoreni instance dekoderu
typedef CVideoDecoder *create_decoder_t();

/// Vypsani napovedy dekoderu na standardni vystup
typedef void print_help_t();

#endif // _VIDEODECODER_H_
