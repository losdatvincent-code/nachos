/* test_sem.c : tests des semaphores (TP 1)
 *
 *  1. le compteur : P() bloque seulement quand il n'y a plus de jeton
 *  2. rendez-vous entre deux threads
 *  3. producteurs / consommateurs avec un tampon de taille fixe
 *
 *  rappel : l'ordonnanceur de Nachos est FIFO, sans partage de temps.
 *  un thread ne perd le processeur que s'il se bloque ou s'il appelle
 *  Yield(). on met donc des Yield() au milieu des sections critiques,
 *  pour obliger les threads a s'entremeler : sans synchronisation
 *  correcte, les tests echoueraient.
 */

#include "userlib/libnachos.h"
#include "userlib/syscall.h"

#define ERR (-1)

int nb_echecs = 0;

// affiche [OK] si la condition est vraie, [ECHEC] sinon
void verifier(int condition, char *message) {
  if (condition) {
    n_printf("  [OK]     %s\n", message);
  } else {
    n_printf("  [ECHEC]  %s\n", message);
    nb_echecs++;
  }
}

// laisse passer les autres threads n fois
void laisser_passer(int n) {
  int i;
  for (i = 0; i < n; i++)
    Yield();
}

/* ---------------------------------------------------------------- */
/* 1. le compteur                                                   */
/* ---------------------------------------------------------------- */

SemId sem;
int passe = 0;   // passe a 1 quand le thread a reussi son P()

void thread_qui_fait_P() {
  P(sem);
  passe = 1;
}

void test_compteur() {
  n_printf("-- Compteur du semaphore\n");
  sem = SemCreate("sem", 2);   // 2 jetons au depart
  verifier((long) sem != ERR, "SemCreate(2)");

  // avec 2 jetons, deux P() passent sans bloquer
  verifier(P(sem) == 0, "premier P() ne bloque pas");
  verifier(P(sem) == 0, "second P() ne bloque pas");

  // un troisieme P(), fait par un autre thread, doit bloquer
  ThreadId t = threadCreate("fait_P", thread_qui_fait_P);
  laisser_passer(3);
  verifier(passe == 0, "le troisieme P() bloque");

  // un V() doit le debloquer
  V(sem);
  laisser_passer(3);
  verifier(passe == 1, "le V() debloque le thread");
  Join(t);

  verifier(SemDestroy(sem) == 0, "SemDestroy");
}

/* ---------------------------------------------------------------- */
/* 2. rendez-vous                                                   */
/* ---------------------------------------------------------------- */

// chaque thread previent l'autre qu'il est arrive (V), puis attend
// que l'autre soit arrive (P). aucun des deux ne doit continuer seul.

SemId arrivee_a, arrivee_b;
int nb_arrives = 0;
int erreur_rdv = 0;

void thread_a() {
  laisser_passer(5);    // A arrive en retard
  nb_arrives++;
  V(arrivee_a);         // je previens B que je suis la
  P(arrivee_b);         // j'attends B
  if (nb_arrives != 2)  // B doit etre arrive
    erreur_rdv = 1;
}

void thread_b() {
  nb_arrives++;
  V(arrivee_b);
  P(arrivee_a);
  if (nb_arrives != 2)
    erreur_rdv = 1;
}

void test_rendez_vous() {
  n_printf("-- Rendez-vous entre deux threads\n");
  arrivee_a = SemCreate("arrivee_a", 0);
  arrivee_b = SemCreate("arrivee_b", 0);
  ThreadId ta = threadCreate("A", thread_a);
  ThreadId tb = threadCreate("B", thread_b);
  Join(ta);
  Join(tb);
  verifier(nb_arrives == 2 && erreur_rdv == 0,
           "aucun thread ne passe le rendez-vous sans l'autre");
  SemDestroy(arrivee_a);
  SemDestroy(arrivee_b);
}

/* ---------------------------------------------------------------- */
/* 3. producteurs / consommateurs                                   */
/* ---------------------------------------------------------------- */

#define TAILLE_TAMPON 3
#define NB_PRODUCTEURS 2
#define NB_CONSOMMATEURS 3
#define NB_PRODUITS 30   // par producteur

int tampon[TAILLE_TAMPON];
int pos_ecriture = 0, pos_lecture = 0;
int nb_dans_tampon = 0;

SemId places_libres;   // nombre de cases vides
SemId places_pleines;  // nombre de cases remplies
SemId mutex;           // un seul thread a la fois dans le tampon

int somme_produite = 0, somme_consommee = 0, nb_consommes = 0;
int nb_dans_section_critique = 0;
int erreur_mutex = 0, erreur_tampon = 0;
int numero_producteur = 0;

void producteur() {
  int k;
  int mon_numero = numero_producteur++;   // pas de Yield ici, pas de conflit

  for (k = 1; k <= NB_PRODUITS; k++) {
    int valeur = (mon_numero + 1) * 1000 + k;   // valeurs toutes differentes

    P(places_libres);
    P(mutex);
    nb_dans_section_critique++;
    if (nb_dans_section_critique != 1)
      erreur_mutex = 1;

    tampon[pos_ecriture] = valeur;
    Yield();   // sans mutex, un autre producteur ecraserait la case
    pos_ecriture = (pos_ecriture + 1) % TAILLE_TAMPON;
    nb_dans_tampon++;
    if (nb_dans_tampon > TAILLE_TAMPON)
      erreur_tampon = 1;
    somme_produite += valeur;

    nb_dans_section_critique--;
    V(mutex);
    V(places_pleines);
  }
}

void consommateur() {
  int k;
  for (k = 0; k < NB_PRODUCTEURS * NB_PRODUITS / NB_CONSOMMATEURS; k++) {
    P(places_pleines);
    P(mutex);
    nb_dans_section_critique++;
    if (nb_dans_section_critique != 1)
      erreur_mutex = 1;

    int valeur = tampon[pos_lecture];
    Yield();
    pos_lecture = (pos_lecture + 1) % TAILLE_TAMPON;
    nb_dans_tampon--;
    if (nb_dans_tampon < 0)
      erreur_tampon = 1;
    somme_consommee += valeur;
    nb_consommes++;

    nb_dans_section_critique--;
    V(mutex);
    V(places_libres);
    Yield();
  }
}

void test_producteurs_consommateurs() {
  int i, k;
  ThreadId threads[NB_PRODUCTEURS + NB_CONSOMMATEURS];

  n_printf("-- Producteurs / consommateurs\n");
  places_libres = SemCreate("places_libres", TAILLE_TAMPON);
  places_pleines = SemCreate("places_pleines", 0);
  mutex = SemCreate("mutex", 1);

  // les consommateurs d'abord : ils se bloquent tout de suite (tampon vide)
  for (i = 0; i < NB_CONSOMMATEURS; i++)
    threads[i] = threadCreate("consommateur", consommateur);
  for (i = 0; i < NB_PRODUCTEURS; i++)
    threads[NB_CONSOMMATEURS + i] = threadCreate("producteur", producteur);
  for (i = 0; i < NB_PRODUCTEURS + NB_CONSOMMATEURS; i++)
    Join(threads[i]);

  // somme attendue de toutes les valeurs produites
  int somme_attendue = 0;
  for (i = 1; i <= NB_PRODUCTEURS; i++)
    for (k = 1; k <= NB_PRODUITS; k++)
      somme_attendue += i * 1000 + k;

  verifier(nb_consommes == NB_PRODUCTEURS * NB_PRODUITS,
           "tout ce qui est produit est consomme");
  verifier(somme_produite == somme_attendue && somme_consommee == somme_attendue,
           "pas de valeur perdue ni lue deux fois");
  verifier(erreur_tampon == 0, "le tampon ne deborde jamais");
  verifier(erreur_mutex == 0, "un seul thread a la fois dans le tampon");

  SemDestroy(places_libres);
  SemDestroy(places_pleines);
  SemDestroy(mutex);
}

int main() {
  n_printf("=== test_sem ===\n");
  test_compteur();
  test_rendez_vous();
  test_producteurs_consommateurs();
  if (nb_echecs == 0)
    n_printf("=== test_sem : tous les tests passent ===\n");
  else
    n_printf("=== test_sem : %d ECHEC(S) ===\n", nb_echecs);
  return 0;
}
