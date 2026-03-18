#!/bin/bash

BIN=./test_simulator

if [ ! -f "$BIN" ]; then
    echo "Ejecutable test_simulator no encontrado. Ejecuta: make test"
    exit 1
fi

pass() { echo -e "   PASS"; }
fail() { echo -e "   FAIL (got $1, expected $2)"; }

echo "==============================="
echo "  TEST AUTOMÁTICOS SIMULADOR"
echo "==============================="

# -------------------------------
# TEST A — DM + ConflictDM
# -------------------------------
echo -n "Test A: DM + ConflictDM (10 accesos)... "
OUT=$($BIN DM LRU ConflictDM 10)
HITS=$(echo "$OUT" | grep "Hits:" | awk '{print $2}')
MISSES=$(echo "$OUT" | grep "Misses:" | awk '{print $2}')
REPL=$(echo "$OUT" | grep "Replacements:" | awk '{print $2}')

# En DM replacement SIEMPRE debe ser 0
if [ "$HITS" -eq 0 ] && [ "$MISSES" -eq 10 ] && [ "$REPL" -eq 0 ]; then
    pass
else
    fail "$HITS/$MISSES/$REPL" "0/10/0"
fi

# -------------------------------
# TEST B — DM + Sequential
# -------------------------------
echo -n "Test B: DM + Sequential (2000 accesos)... "
OUT=$($BIN DM LRU Sequential 2000)
REPL=$(echo "$OUT" | grep "Replacements:" | awk '{print $2}')

# En DM replacement SIEMPRE debe ser 0
if [ "$REPL" -eq 0 ]; then
    pass
else
    fail "$REPL" "0"
fi

# -------------------------------
# TEST C — DM ignora política
# -------------------------------
echo -n "Test C: DM ignora política... "
OUT1=$($BIN DM LRU Random 2000)
OUT2=$($BIN DM FIFO Random 2000)
OUT3=$($BIN DM Random Random 2000)

SIG1=$(echo "$OUT1" | md5sum | awk '{print $1}')
SIG2=$(echo "$OUT2" | md5sum | awk '{print $1}')
SIG3=$(echo "$OUT3" | md5sum | awk '{print $1}')

if [ "$SIG1" = "$SIG2" ] && [ "$SIG2" = "$SIG3" ]; then
    pass
else
    fail "diferentes" "idénticos"
fi

# -------------------------------
# TEST D — FA + Random
# -------------------------------
echo -n "Test D: FA + Random (2000 accesos)... "
OUT=$($BIN FA LRU Random 2000)
REPL=$(echo "$OUT" | grep "Replacements:" | awk '{print $2}')

if [ "$REPL" -gt 0 ]; then
    pass
else
    fail "$REPL" ">0"
fi

# -------------------------------
# TEST E — 2W + ConflictDM
# -------------------------------
echo -n "Test E: 2W + ConflictDM (10 accesos)... "
OUT=$($BIN 2W LRU ConflictDM 10)
HITS=$(echo "$OUT" | grep "Hits:" | awk '{print $2}')

# En 2W, ConflictDM debe forzar misses (0 hits)
if [ "$HITS" -eq 0 ]; then
    pass
else
    fail "$HITS" "0"
fi

# -------------------------------
# TEST F — FA + FourStrike
# -------------------------------
echo -n "Test F: FA + FourStrike (100 accesos)... "
OUT1=$($BIN FA LRU FourStrike 100)
OUT2=$($BIN FA FIFO FourStrike 100)
OUT3=$($BIN FA Random FourStrike 100)

SIG1=$(echo "$OUT1" | md5sum | awk '{print $1}')
SIG2=$(echo "$OUT2" | md5sum | awk '{print $1}')
SIG3=$(echo "$OUT3" | md5sum | awk '{print $1}')

# Las políticas deben dar resultados distintos
if [ "$SIG1" != "$SIG2" ] || [ "$SIG1" != "$SIG3" ]; then
    pass
else
    fail "idénticos" "diferentes"
fi
