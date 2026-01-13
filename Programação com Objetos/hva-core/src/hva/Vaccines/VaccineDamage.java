package hva.Vaccines;

/**
 * Enum representing the various types of vaccine damage.
 * Each type includes a range of damage values and a description.
 */
public enum VaccineDamage {
    MESMA_ESPECIE(0, "NORMAL"),
    ESPECIES_DIFERNTES(0, "CONFUSAO"),
    ACIDENTE(1,4, "ACIDENTE"),
    ERRO(5, Integer.MAX_VALUE, "ERRO");

    private final int _minDano;
    private final int _maxDano;
    private final String _descricao;

    /**
     * Constructor for creating a VaccineDamage enum constant with a single damage value.
     *
     * @param minDano   the minimum damage value (and also the maximum for single value)
     * @param descricao the description of the damage type
     */
    VaccineDamage(int minDano, String descricao) {
        _minDano = minDano;
        _maxDano = minDano;
        _descricao = descricao;
    }

    /**
     * Constructor for creating a VaccineDamage enum constant with a range of damage values.
     *
     * @param minDano   the minimum damage value
     * @param maxDano   the maximum damage value
     * @param descricao the description of the damage type
     */
    VaccineDamage(int minDano, int maxDano, String descricao) {
        _minDano = minDano;
        _maxDano = maxDano;
        _descricao = descricao;
    }

    /**
     * Returns the description of the damage type based on the provided damage value.
     *
     * @param dano the damage value to evaluate
     * @return the description of the damage type, or "DESCONHECIDO" if no match is found
     */
    public String getDamage(int dano) {
        for (VaccineDamage _d: VaccineDamage.values()) {
            if (dano >= _d._minDano && dano <= _d._maxDano) return _d._descricao;
        }

        return "DESCONHECIDO";
    }

}