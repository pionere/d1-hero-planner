/**
 * @file defs.h
 *
 * Global definitions and Macros.
 */
#ifndef _DEFS_H
#define _DEFS_H

//#undef DEV_MODE
#define DEV_MODE 1
#define HELLFIRE 1

#define DEVILUTION_BEGIN_NAMESPACE
#define DEVILUTION_END_NAMESPACE
#define ALIGN32
#define ALIGN64
#define ALIGNMENT
#define ALIGN

#define INV_SLOT_SIZE_PX 28

#define SwapLE16(X) qToLittleEndian((quint16)(X))
#define SwapLE32(X) qToLittleEndian((quint32)(X))
#define SwapLE64(X) qToLittleEndian((quint64)(X))

// MAXDUN = DSIZE + 2 * DBORDER
// DSIZE = 2 * DMAX
#define DMAXX                    40
#define DMAXY                    40
#define DBORDERX                 16
#define DBORDERY                 16
#define DSIZEX                   80
#define DSIZEY                   80
#define MAXDUNX                  112
#define MAXDUNY                  112
/** The size of the quads in hell. */
static_assert(DMAXX % 2 == 0, "DRLG_L4 constructs the dungeon by mirroring a quarter block -> requires to have a dungeon with even width.");
#define L4BLOCKX (DMAXX / 2)
static_assert(DMAXY % 2 == 0, "DRLG_L4 constructs the dungeon by mirroring a quarter block -> requires to have a dungeon with even height.");
#define L4BLOCKY (DMAXY / 2)

// must be unsigned to generate unsigned comparisons with pnum
#define MAX_PLRS                 4
#define MAX_MINIONS              MAX_PLRS

#define MAX_LVLMTYPES            12
#define MAX_LVLMIMAGE            4000

#ifdef HELLFIRE
#define MAXTRIGGERS              7
#else
#define MAXTRIGGERS              5
#endif

#define DEAD_MULTI              0xFF
#define MAXITEMS                127
#define ITEM_NONE               0xFF
#define MAXBELTITEMS            8
//#define MAXLIGHTS               32
#define MAXLIGHTS               0
#define MAXMONSTERS             200
#define MON_NONE                0xFF
#define MAXOBJECTS              127
#define OBJ_NONE                0xFF
#define MAXTHEMES               8
#define MAXTILES                255
#define MAXSUBTILES             1023
#define MAXVISION               (MAX_PLRS + MAX_MINIONS)
#define MAXCHARLEVEL            50
#define MAXSPLLEVEL             15
#ifdef HELLFIRE
#define BASESTAFFCHARGES        18
#else
#define BASESTAFFCHARGES        40
#endif

#define MAXCAMPAIGNLVL          60
#define MAXCAMPAIGNSIZE         16

// number of inventory grid cells
#define NUM_INV_GRID_ELEM       40

// Item indestructible durability
#define DUR_INDESTRUCTIBLE      255

// from diablo 2 beta
#define MAXRESIST               75
#define PLR_MIN_VISRAD          10

#define GOLD_SMALL_LIMIT        1000
#define GOLD_MEDIUM_LIMIT       2500
#define GOLD_MAX_LIMIT          5000

#define PLR_NAME_LEN            32

#define MAXPATHNODES            256

#define MAX_PATH_LENGTH         23

// Diablo uses a 256 color palette
// Entry 0-127 (0x00-0x7F) are level specific
// Entry 128-255 (0x80-0xFF) are global
#define NUM_COLORS        256

// standard palette for all levels
// 8 or 16 shades per color
// example (dark blue): PAL16_BLUE+14, PAL8_BLUE+7
// example (light red): PAL16_RED+2, PAL8_RED
// example (orange): PAL16_ORANGE+8, PAL8_ORANGE+4
#define PAL8_BLUE         128
#define PAL8_RED          136
#define PAL8_YELLOW       144
#define PAL8_ORANGE       152
#define PAL16_BEIGE       160
#define PAL16_BLUE        176
#define PAL16_YELLOW      192
#define PAL16_ORANGE      208
#define PAL16_RED         224
#define PAL16_GRAY        240

#define NIGHTMARE_LEVEL_BONUS   16
#define HELL_LEVEL_BONUS        32

#define POS_IN_RECT(x, y, rx, ry, rw, rh) \
    ((x) >= (rx)                          \
    && (x) < (rx + rw)                    \
    && (y) >= (ry)                        \
    && (y) < (ry + rh))

#define IN_DUNGEON_AREA(x, y) \
    (x >= 0                   \
    && x < MAXDUNX            \
    && y >= 0                 \
    && y < MAXDUNY)

#ifndef TRUE
#define TRUE true
#endif
#ifndef FALSE
#define FALSE false
#endif

#ifdef _MSC_VER
#define ASSUME_UNREACHABLE __assume(0);
#elif defined(__clang__)
#define ASSUME_UNREACHABLE __builtin_unreachable();
#elif defined(__GNUC__)
#if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 405
#define ASSUME_UNREACHABLE __builtin_unreachable();
#else
#define ASSUME_UNREACHABLE assert(0);
#endif
#endif

#if DEBUG_MODE || DEV_MODE
#undef ASSUME_UNREACHABLE
#define ASSUME_UNREACHABLE assert(0);
#endif

#if DEBUG_MODE || DEV_MODE
//#define dev_assert(exp, msg, ...) (void)((exp) || (app_fatal(msg, __VA_ARGS__), 0))
#define dev_assert(exp, msg, ...) assert(exp)
#else
#define dev_assert(exp, msg, ...) ((void)0)
#endif

#if INET_MODE
#define net_assert(x) assert(x)
#else
#define net_assert(x) do { } while(0)
#endif

#endif /* _DEFS_H */
