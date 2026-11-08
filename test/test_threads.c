/* test_threads.c : gestion des threads (TP 1)
 *
 *  teste Thread::Start, Finish, SwitchTo, Join et la sauvegarde /
 *  restauration des registres :
 *  1. beaucoup de threads crees puis termines l'un apres l'autre
 *     (verifie que les threads termines sont bien detruits sans casser
 *     le systeme)
 *  2. plusieurs threads en meme temps : chacun garde ses propres
 *     variables locales (sa pile) malgre les changements de thread
 *  3. Join sur un thread deja termine, ou sur un identifiant invalide
 *  4. un thread qui cree lui-meme un thread
 */

#include "userlib/libnachos.h"
#include "userlib/syscall.h"

#define ERR (-1)

int nb_echecs = 0;

void verifier(int condition, char *message) {
  if (condition) {
    n_printf("  [OK]     %s\n", message);
  } else {
    n_printf("  [ECHEC]  %s\n", message);
    nb_echecs++;
  }
}

/* ---------------------------------------------------------------- */
/* 1. threads successifs                                            */
/* ---------------------------------------------------------------- */

#define NB_SUCCESSIFS 20

int compteur = 0;

void incremente() {
  compteur++;
}

void test_threads_successifs() {
  int i;
  n_printf("-- %d threads crees et termines l'un apres l'autre\n", NB_SUCCESSIFS);
  for (i = 0; i < NB_SUCCESSIFS; i++) {
    ThreadId t = threadCreate("incremente", incremente);
    Join(t);
  }
  verifier(compteur == NB_SUCCESSIFS, "chaque thread s'est execute une fois");
}

/* ---------------------------------------------------------------- */
/* 2. threads simultanes                                            */
/* ---------------------------------------------------------------- */

#define NB_SIMULTANES 6
#define NB_ETAPES 10

int numero = 0;
int resultats[NB_SIMULTANES];

void calcule() {
  int mon_numero = numero++;
  int somme = 0;   // variable locale : elle est sur la pile de ce thread
  int i;
  for (i = 1; i <= NB_ETAPES; i++) {
    somme = somme + mon_numero * 100 + i;
    Yield();       // on passe la main aux autres a chaque etape
  }
  resultats[mon_numero] = somme;
}

void test_threads_simultanes() {
  int i;
  int tout_est_bon = 1;
  ThreadId threads[NB_SIMULTANES];

  n_printf("-- %d threads en meme temps\n", NB_SIMULTANES);
  for (i = 0; i < NB_SIMULTANES; i++)
    threads[i] = threadCreate("calcule", calcule);
  for (i = 0; i < NB_SIMULTANES; i++)
    Join(threads[i]);

  // le thread numero i doit avoir trouve 100*i*NB_ETAPES + (1+2+...+NB_ETAPES)
  for (i = 0; i < NB_SIMULTANES; i++)
    if (resultats[i] != 100 * i * NB_ETAPES + NB_ETAPES * (NB_ETAPES + 1) / 2)
      tout_est_bon = 0;
  verifier(tout_est_bon, "chaque thread garde ses variables locales");
}

/* ---------------------------------------------------------------- */
/* 3. Join                                                          */
/* ---------------------------------------------------------------- */

void test_join() {
  n_printf("-- Join\n");
  ThreadId t = threadCreate("incremente", incremente);
  Join(t);
  verifier(Join(t) == 0, "Join sur un thread deja termine ne bloque pas");
  verifier(Join(987654) == 0, "Join sur un identifiant invalide ne bloque pas");
}

/* ---------------------------------------------------------------- */
/* 4. thread cree par un thread                                     */
/* ---------------------------------------------------------------- */

int petit_fils_fini = 0;

void petit_fils() {
  petit_fils_fini = 1;
}

void fils() {
  ThreadId t = threadCreate("petit_fils", petit_fils);
  Join(t);
}

void test_thread_cree_par_un_thread() {
  n_printf("-- Thread cree par un autre thread\n");
  ThreadId t = threadCreate("fils", fils);
  Join(t);
  verifier(petit_fils_fini == 1, "le petit-fils s'est bien execute");
}

int main() {
  n_printf("=== test_threads ===\n");
  test_threads_successifs();
  test_threads_simultanes();
  test_join();
  test_thread_cree_par_un_thread();
  if (nb_echecs == 0)
    n_printf("=== test_threads : tous les tests passent ===\n");
  else
    n_printf("=== test_threads : %d ECHEC(S) ===\n", nb_echecs);
  return 0;
}
