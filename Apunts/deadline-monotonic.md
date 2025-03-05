# Deadline Monotonic

Condicions del Rate Monotonic **no serveixen**.  
S'ha de fer el response-time analysis.

```rust
Ci = "Temps comput tasca"
Wi^n = "resultat anterior (comença amb 0)"
Tj = "Periode tasca interfereix"
Cj = "Temps comput tasca interfereix"
Wi^n+1 = Ci + SumatoriInterferencies(Ceiling(Wi^n/Tj) * Cj)

// Comprovar si Wi^n+1 <= Deadline

// Repetir fins que Wi^n == Wi^n+1 
```