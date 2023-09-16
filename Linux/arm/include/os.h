/*
 * Linux arm
 */

/*
 * This structure must agree with FPsave and FPrestore asm routines
 */

#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <grp.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pwd.h>


typedef struct FPU FPU;
struct FPU
{
        uchar	env[28];
};

#define	DIRTYPE	struct dirent

/*
extern	Proc*	uptab[];
*/

extern Proc *getup(void);

#define up (getup())
