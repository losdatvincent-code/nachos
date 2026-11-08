/* test_rw.c : lecteurs / redacteurs avec des semaphores (TP 1)
 *
 *  solution classique du cours, avec priorite aux lecteurs :
 *   - plusieurs lecteurs peuvent lire en meme temps,
 *   - un redacteur doit etre seul (ni lecteur, ni autre redacteur).
 */

#include "userlib/libnachos.h"
#include "userlib/syscall.h"

#define NB_LECTEURS 3
#define NB_REDACTEURS 2
#define NB_TOURS 5

int nb_echecs = 0;

void verifier(int condition, char *message) {
  if (condition) {
    n_printf("  [OK]     %s\n", message);
  } else {
    n_printf("  [ECHEC]  %s\n", message);
    nb_echecs++;
  }
}

SemId mutex_nb_lecteurs;   // protege nb_lecteurs
SemId acces_donnee;        // pris par un redacteur, ou par le groupe de lecteurs
int nb_lecteurs = 0;       // lecteurs en train de lire

int donnee = 0;            // la donnee partagee

// pour verifier que tout se passe bien
int lecteurs_dedans = 0, redacteurs_dedans = 0;
int max_lecteurs_dedans = 0;
int nb_lectures = 0;
int erreur = 0;

void lecteur() {
  int tour;
  for (tour = 0; tour < NB_TOURS; tour++) {
    // entree : le premier lecteur bloque les redacteurs
    P(mutex_nb_lecteurs);
    nb_lecteurs++;
    if (nb_lecteurs == 1)
      P(acces_donnee);
    V(mutex_nb_lecteurs);

    // lecture
    lecteurs_dedans++;
    if (lecteurs_dedans > max_lecteurs_dedans)
      max_lecteurs_dedans = lecteurs_dedans;
    if (redacteurs_dedans != 0)
      erreur = 1;
    int valeur_lue = donnee;
    Yield();
    Yield();
    if (donnee != valeur_lue)   // quelqu'un a ecrit pendant qu'on lisait
      erreur = 1;
    lecteurs_dedans--;
    nb_lectures++;

    // sortie : le dernier lecteur laisse passer les redacteurs
    P(mutex_nb_lecteurs);
    nb_lecteurs--;
    if (nb_lecteurs == 0)
      V(acces_donnee);
    V(mutex_nb_lecteurs);
    Yield();
  }
}

void redacteur() {
  int tour;
  for (tour = 0; tour < NB_TOURS; tour++) {
    P(acces_donnee);

    redacteurs_dedans++;
    if (redacteurs_dedans != 1 || lecteurs_dedans != 0)
      erreur = 1;
    int copie = donnee;
    Yield();
    donnee = copie + 1;
    redacteurs_dedans--;

    V(acces_donnee);
    Yield();
  }
}

int main() {
  int i;
  ThreadId threads[NB_LECTEURS + NB_REDACTEURS];

  n_printf("=== test_rw ===\n");
  mutex_nb_lecteurs = SemCreate("mutex_nb_lecteurs", 1);
  acces_donnee = SemCreate("acces_donnee", 1);

  // on melange lecteurs et redacteurs a la creation
  threads[0] = threadCreate("lecteur", lecteur);
  threads[1] = threadCreate("redacteur", redacteur);
  threads[2] = threadCreate("lecteur", lecteur);
  threads[3] = threadCreate("redacteur", redacteur);
  threads[4] = threadCreate("lecteur", lecteur);
  for (i = 0; i < NB_LECTEURS + NB_REDACTEURS; i++)
    Join(threads[i]);

  verifier(erreur == 0, "un redacteur est toujours seul");
  verifier(donnee == NB_REDACTEURS * NB_TOURS, "toutes les ecritures sont faites");
  verifier(nb_lectures == NB_LECTEURS * NB_TOURS, "toutes les lectures sont faites");
  verifier(max_lecteurs_dedans > 1, "plusieurs lecteurs ont lu en meme temps");

  SemDestroy(mutex_nb_lecteurs);
  SemDestroy(acces_donnee);
  if (nb_echecs == 0)
    n_printf("=== test_rw : tous les tests passent ===\n");
  else
    n_printf("=== test_rw : %d ECHEC(S) ===\n", nb_echecs);
  return 0;
}
