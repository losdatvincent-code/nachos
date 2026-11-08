/* test_lock.c : tests des verrous (TP 1)
 *
 *  1. exclusion mutuelle sur un compteur partage
 *  2. verrou reentrant : le proprietaire peut reprendre son verrou, et
 *     le verrou n'est libere qu'au dernier Release
 *  3. seul le proprietaire peut relacher le verrou
 *  4. les threads en attente obtiennent le verrou dans l'ordre d'arrivee
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

void laisser_passer(int n) {
  int i;
  for (i = 0; i < n; i++)
    Yield();
}

/* ---------------------------------------------------------------- */
/* 1. exclusion mutuelle                                            */
/* ---------------------------------------------------------------- */

#define NB_THREADS 4
#define NB_INCREMENTS 25

LockId verrou_compteur;
int compteur = 0;

void incrementeur() {
  int i;
  for (i = 0; i < NB_INCREMENTS; i++) {
    LockAcquire(verrou_compteur);
    int copie = compteur;
    Yield();   // sans verrou, les autres liraient la meme valeur que nous
               // et des increments seraient perdus
    compteur = copie + 1;
    LockRelease(verrou_compteur);
    Yield();
  }
}

void test_exclusion_mutuelle() {
  int i;
  ThreadId threads[NB_THREADS];

  n_printf("-- Exclusion mutuelle\n");
  verrou_compteur = LockCreate("compteur");
  verifier((long) verrou_compteur != ERR, "LockCreate");
  for (i = 0; i < NB_THREADS; i++)
    threads[i] = threadCreate("incrementeur", incrementeur);
  for (i = 0; i < NB_THREADS; i++)
    Join(threads[i]);
  verifier(compteur == NB_THREADS * NB_INCREMENTS, "aucun increment perdu");
  verifier(LockDestroy(verrou_compteur) == 0, "LockDestroy");
}

/* ---------------------------------------------------------------- */
/* 2. reentrance                                                    */
/* ---------------------------------------------------------------- */

LockId verrou_reentrant;
int autre_a_le_verrou = 0;

void autre_thread() {
  LockAcquire(verrou_reentrant);
  autre_a_le_verrou = 1;
  LockRelease(verrou_reentrant);
}

void test_reentrance() {
  n_printf("-- Verrou reentrant\n");
  verrou_reentrant = LockCreate("reentrant");

  LockAcquire(verrou_reentrant);
  // si le verrou n'etait pas reentrant, on resterait bloque ici pour toujours
  verifier(LockAcquire(verrou_reentrant) == 0,
           "le proprietaire reprend son verrou sans se bloquer");

  ThreadId t = threadCreate("autre", autre_thread);
  laisser_passer(3);
  verifier(autre_a_le_verrou == 0, "un autre thread est bloque");

  LockRelease(verrou_reentrant);   // 1er Release : il reste un Acquire
  laisser_passer(3);
  verifier(autre_a_le_verrou == 0, "toujours bloque apres le premier Release");

  LockRelease(verrou_reentrant);   // 2e Release : le verrou est libere
  laisser_passer(3);
  verifier(autre_a_le_verrou == 1, "debloque apres le second Release");

  Join(t);
  LockDestroy(verrou_reentrant);
}

/* ---------------------------------------------------------------- */
/* 3. seul le proprietaire peut relacher                            */
/* ---------------------------------------------------------------- */

LockId verrou_proprio;
int resultat_intrus;

void intrus() {
  resultat_intrus = LockRelease(verrou_proprio);   // il n'a pas le verrou
}

void test_proprietaire() {
  n_printf("-- Seul le proprietaire peut relacher\n");
  verrou_proprio = LockCreate("proprio");

  verifier(LockRelease(verrou_proprio) == ERR,
           "relacher un verrou libre est refuse");

  LockAcquire(verrou_proprio);
  ThreadId t = threadCreate("intrus", intrus);
  Join(t);
  verifier(resultat_intrus == ERR, "un autre thread ne peut pas le relacher");
  verifier(LockRelease(verrou_proprio) == 0, "le proprietaire peut le relacher");

  LockDestroy(verrou_proprio);
}

/* ---------------------------------------------------------------- */
/* 4. ordre d'arrivee                                               */
/* ---------------------------------------------------------------- */

#define NB_ATTENTE 4

LockId verrou_fifo;
int ordre[NB_ATTENTE];   // ordre[i] = numero du i-eme thread servi
int nb_servis = 0;
int numero = 0;

void thread_en_attente() {
  int mon_numero = numero++;
  LockAcquire(verrou_fifo);
  ordre[nb_servis] = mon_numero;
  nb_servis++;
  LockRelease(verrou_fifo);
}

void test_ordre() {
  int i;
  int dans_l_ordre = 1;
  ThreadId threads[NB_ATTENTE];

  n_printf("-- Ordre d'arrivee\n");
  verrou_fifo = LockCreate("fifo");
  LockAcquire(verrou_fifo);
  for (i = 0; i < NB_ATTENTE; i++)
    threads[i] = threadCreate("en_attente", thread_en_attente);
  laisser_passer(NB_ATTENTE + 1);   // ils se bloquent tous, dans l'ordre
  verifier(nb_servis == 0, "tous les threads attendent");

  LockRelease(verrou_fifo);
  for (i = 0; i < NB_ATTENTE; i++)
    Join(threads[i]);
  for (i = 0; i < NB_ATTENTE; i++)
    if (ordre[i] != i)
      dans_l_ordre = 0;
  verifier(nb_servis == NB_ATTENTE && dans_l_ordre,
           "le verrou est donne dans l'ordre d'arrivee");

  LockDestroy(verrou_fifo);
}

int main() {
  n_printf("=== test_lock ===\n");
  test_exclusion_mutuelle();
  test_reentrance();
  test_proprietaire();
  test_ordre();
  if (nb_echecs == 0)
    n_printf("=== test_lock : tous les tests passent ===\n");
  else
    n_printf("=== test_lock : %d ECHEC(S) ===\n", nb_echecs);
  return 0;
}
