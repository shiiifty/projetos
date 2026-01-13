package hva.Animal;

import java.io.Serializable;

/**
 * Represents a species of animal in the system.
 * Implements Serializable to allow species objects to be serialized.
 */
public class Specie implements Serializable {
    private String _specieId;   // Unique identifier for the species
    private String _specieName; // Name of the species

    /**
     * Constructs a Specie object with a specified ID and name.
     *
     * @param id   the unique identifier for the species
     * @param name the name of the species
     */
    public Specie(String id, String name) {
        _specieId = id;
        _specieName = name;
    }

    /**
     * Returns the unique identifier of the species.
     *
     * @return the species ID
     */
    public String getSpecieId() {
        return _specieId;
    }

    /**
     * Returns the name of the species.
     *
     * @return the species name
     */
    public String getSpecieName() {
        return _specieName;
    }
}