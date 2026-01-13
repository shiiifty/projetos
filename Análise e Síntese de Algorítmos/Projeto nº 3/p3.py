import sys
import pulp

def main():
    input_data = sys.stdin.read().splitlines()
    n, m, t = map(int, input_data[0].split())

    factories_country = {}
    fmax = {}
    for i in range(1, n + 1):
        i, j, s = map(int, input_data[i].split())
        if s > 0:
            factories_country[i] = j
            fmax[i] = s

    p_max = {}
    p_min = {}
    for i in range(n + 1, n + m + 1):
        j, mx, mn = map(int, input_data[i].split())
        p_max[j] = mx
        p_min[j] = mn

    child_country = {}
    child_requests = {}
    for i in range(n + m + 1, n + m + t + 1):
        parts = list(map(int, input_data[i].split()))
        k = parts[0]
        c = parts[1]
        child_country[k] = c
        child_requests[k] = [r for r in parts[2:] if r in factories_country]

    model = pulp.LpProblem("natal_brinquedos", pulp.LpMaximize)

    x = {}
    for k, facs in child_requests.items():
        for f in facs:
            x[(k,f)] = pulp.LpVariable(f"x_{k}_{f}", cat=pulp.LpBinary)

    model += pulp.lpSum(x.values())

    for k, facs in child_requests.items():
        model += pulp.lpSum(x[(k,f)] for f in facs if (k,f) in x) <= 1

    for f in fmax:
        model += pulp.lpSum(x[(k,f2)] for (k,f2) in x if f2 == f) <= fmax[f]

    for (j_min, j_max) in zip(p_min, p_max):
        model += pulp.lpSum(x[(k, f2)] for (k, f2) in x if child_country[k] == j_min) >= p_min[j_min]
        model += pulp.lpSum(x[(k, f2)] for (k, f2) in x if factories_country[f2] == j_max and child_country[k] != j_max) <= p_max[j_max]

    r = model.solve(pulp.GLPK_CMD(msg=0))
    
    if pulp.LpStatus[r] == "Infeasible":
        print(-1)
    else:
        print(int(pulp.value(model.objective)))

if __name__ == "__main__":
    main()
