package hva.trees;

/**
 * Enum representing the types of trees and their associated care effort for different seasons.
 */
public enum TreeType {
    CADUCA(0, 1, 2, 5),
    PERENE(2, 1, 1, 1);

    private final int _esforcoInverno;
    private final int _esforcoPrimavera;
    private final int _esforcoVerao;
    private final int _esforcoOutono;

    /**
     * Constructor for creating a TreeType enum constant with specified care efforts for each season.
     *
     * @param esforcoInverno    the care effort required in winter
     * @param esforcoPrimavera  the care effort required in spring
     * @param esforcoVerao     the care effort required in summer
     * @param esforcoOutono     the care effort required in autumn
     */
    TreeType(int esforcoInverno, int esforcoPrimavera, int esforcoVerao, int esforcoOutono) {
        _esforcoInverno = esforcoInverno;
        _esforcoPrimavera = esforcoPrimavera;
        _esforcoVerao = esforcoVerao;
        _esforcoOutono = esforcoOutono;
    }

    /**
     * Returns the care effort required for the specified season.
     *
     * @param season the season to evaluate (INVERNO, PRIMAVERA, VERAO, OUTONO)
     * @return the care effort required for the specified season
     * @throws IllegalArgumentException if the provided season is invalid
     */
    public int getEffort(String season) {
        switch (season) {
            case "INVERNO":
                return _esforcoInverno;
            case "PRIMAVERA":
                return _esforcoPrimavera;

            case "VERAO":
                return _esforcoVerao;
                
            case "OUTONO":
                return _esforcoOutono;       
            
            default:
                throw new IllegalArgumentException("Invalid season:" + season);
        }
    }
}