#!/bin/sh
# lance les programmes de test du TP 1 (apres avoir fait make)
#   ./tp1_tests.sh       affiche le bilan de chaque test
#   ./tp1_tests.sh -v    affiche aussi le detail

echo "tests normaux (tout doit passer) :"
for t in test_sem test_rw test_threads
do
  if [ "$1" = "-v" ]; then
    ./nachos -f tp1.cfg -x /$t | grep -E '^ *\[|^--|PError'
  fi
  # derniere ligne affichee par le test : "=== test_xxx : ... ==="
  # si elle manque, le test s'est bloque ou nachos s'est arrete avant
  ./nachos -f tp1.cfg -x /$t | grep "^=== $t :" || echo "=== $t : PAS DE BILAN ==="
done
