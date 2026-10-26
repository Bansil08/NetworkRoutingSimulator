#!/bin/bash
# Automated test runner for NetworkRoutingSimulator
# Feeds scripted input and captures output for verification

BIN="${BIN:-./build/NetworkRoutingSimulator}"
DATA="./data/sample_network.txt"
PASS=0
FAIL=0

# run_test NAME INPUT EXPECTED [EXPECTED...]  — every EXPECTED string must appear
run_test() {
    local name="$1"
    local input="$2"
    shift 2
    local output expected
    output=$(printf "$input" | "$BIN" "$DATA" 2>&1)
    for expected in "$@"; do
        if ! echo "$output" | grep -qF -- "$expected"; then
            echo "  [FAIL] $name"
            echo "         Expected to find: '$expected'"
            echo "         --- Actual output snippet ---"
            echo "$output" | tail -20
            ((FAIL++))
            return
        fi
    done
    echo "  [PASS] $name"
    ((PASS++))
}

# run_test_absent NAME INPUT UNEXPECTED  — UNEXPECTED must NOT appear
run_test_absent() {
    local output
    output=$(printf "$2" | "$BIN" "$DATA" 2>&1)
    if echo "$output" | grep -qF -- "$3"; then
        echo "  [FAIL] $1"
        echo "         Did not expect: '$3'"
        ((FAIL++))
    else
        echo "  [PASS] $1"
        ((PASS++))
    fi
}

echo ""
echo "  ══════════════════════════════════════════════"
echo "   AUTOMATED TEST SUITE — NetworkRoutingSimulator"
echo "  ══════════════════════════════════════════════"
echo ""

# ── T1: Dijkstra shortest path 1→8 (cost 17)
run_test "T1: Dijkstra route 1→8, cost=17" \
    "6\n1\n5\n1\n8\n0" \
    "Total Cost: 17"

# ── T2: BFS path 1→8 (3 hops, higher cost 20)
run_test "T2: BFS route 1→8, cost=20 (min-hops)" \
    "6\n2\n5\n1\n8\n0" \
    "Total Cost: 20"

# ── T3: Dijkstra vs BFS give different routes in the same session
run_test "T3: Dijkstra and BFS produce different costs for 1→8" \
    "6\n1\n5\n1\n8\n6\n2\n5\n1\n8\n0" \
    "Total Cost: 17" "Total Cost: 20"

# ── T4: Connectivity check on full network
run_test "T4: Full network is CONNECTED" \
    "9\n0" \
    "FULLY CONNECTED"

# ── T5: Packet delivery success
run_test "T5: Packet delivered successfully" \
    "6\n1\n7\n1\n8\n3\n0" \
    "delivered successfully"

# ── T6: Link failure triggers rerouting (remove echo-golf link 5-7 which is on Dijkstra path)
# First send packet 1→8 (Dijkstra: 1→3→5→7→8), then remove link 5→7
run_test "T6: Link failure reroutes packet" \
    "6\n1\n7\n1\n8\n1\n8\n5\n7\n0" \
    "New Route Found"

# ── T7: Disconnect router 8, then check connectivity → DISCONNECTED
run_test "T7: Disconnected network detected" \
    "3\n7\n8\n3\n5\n8\n3\n6\n8\n9\n0" \
    "DISCONNECTED"

# ── T8: Route to isolated router 8 fails
run_test "T8: No route to isolated router 8" \
    "3\n7\n8\n3\n5\n8\n3\n6\n8\n5\n1\n8\n0" \
    "No route found"

# ── T9: Kruskal MST on connected network
run_test "T9: Kruskal MST found on connected network" \
    "10\n0" \
    "Minimum Cost Network found"

# ── T10: Kruskal MST on disconnected network fails
run_test "T10: MST fails on disconnected network" \
    "3\n7\n8\n3\n5\n8\n3\n6\n8\n10\n0" \
    "Cannot build MST"

# ── T11: Duplicate router ID rejected
run_test "T11: Duplicate router ID rejected" \
    "1\n1\nAlpha2\n0" \
    "already exists"

# ── T12: Self-loop rejected
run_test "T12: Self-loop link rejected" \
    "2\n1\n1\n3\n0" \
    "Self-loops are not allowed"

# ── T13: Negative weight rejected
run_test "T13: Negative weight rejected" \
    "2\n1\n2\n-5\n0" \
    "Negative link weights are not supported"

# ── T14: Duplicate link rejected
run_test "T14: Duplicate link rejected" \
    "2\n1\n2\n99\n0" \
    "already exists"

# ── T15: File save and reload
run_test "T15: Save and reload network" \
    "13\n/tmp/test_save_net.txt\n12\n/tmp/test_save_net.txt\n0" \
    "Network loaded from"

# ── T16: Topology display shows all 8 routers
run_test "T16: Topology shows all routers" \
    "4\n0" \
    "Routers: 8"

# ── T17: Statistics display shows correct packet counts
run_test "T17: Statistics display works" \
    "11\n0" \
    "Total Packets Sent"

# ── T18: Same src/dst returns trivial path
run_test "T18: Source == Destination is handled" \
    "5\n3\n3\n0" \
    "Source and destination are the same"

# ── T19: Route to nonexistent router
run_test "T19: Route to nonexistent router 99 rejected" \
    "5\n1\n99\n0" \
    "not found"

# ── T20: Remove nonexistent link
run_test "T20: Remove nonexistent link rejected" \
    "3\n1\n8\n0" \
    "No link between"

# ── T21: Trailing junk after a number is rejected
run_test "T21: Menu choice '1abc' rejected" \
    "1abc\n0" \
    "Invalid menu choice"

run_test "T22: Router ID '5x' rejected, then re-prompted" \
    "1\n5x\n9\nIndia\n0" \
    "Please enter a valid integer" "Router [9] 'India' added"

# ── T23: Input ending mid-command abandons it instead of using fake values
run_test_absent "T23: EOF mid-prompt does not run the command" \
    "2\n1\n" \
    "Destination router 0 not found"

# ── T24: Router names are trimmed so save/reload round-trips exactly
run_test "T24: Router name whitespace trimmed" \
    "1\n9\n  Lima  \n0" \
    "Router [9] 'Lima' added"

echo ""
echo "  ══════════════════════════════════════════════"
printf  "   Results: %d passed, %d failed (out of %d)\n" "$PASS" "$FAIL" "$((PASS+FAIL))"
echo "  ══════════════════════════════════════════════"
echo ""

[ "$FAIL" -eq 0 ] && exit 0 || exit 1
