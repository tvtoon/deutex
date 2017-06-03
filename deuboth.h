#define END  1
#define NIL  2
#define OPT  3
#define CMD  4

typedef void (*comfun_t) (int argc, char *argv[]);

typedef struct
{
 char type;
 char argc;
 const char *com;
 comfun_t exec;
 const char *use;
 const char *help;
} comdef_t;

void COMwintex(int argc, char *argv[]);
void COMverbose(int argc, char *argv[]);
void COMdoom(int argc, char *argv[]);
void COMstrife(int argc, char *argv[]);
void COMmain(int argc, char *argv[]);
void COMwadir(int argc, char *argv[]);
void COMadd(int argc, char *argv[]);
void COMapp(int argc, char *argv[]);
void COMapps(int argc, char *argv[]);
void COMappf(int argc, char *argv[]);
void COMjoin(int argc, char *argv[]);
void COMmerge(int argc, char *argv[]);
void COMrestor(int argc, char *argv[]);
void COMcheck(int argc, char *argv[]);
void COMhelp (int argc, char *argv[]);
void COMvers (int argc, char *argv[]);
void COMformat (int argc, char *argv[]);
void COMtf (int argc, char *argv[]);
int is_prefix (const char *s1, const char *s2);
void call_opt (comfun_t func, ...);
