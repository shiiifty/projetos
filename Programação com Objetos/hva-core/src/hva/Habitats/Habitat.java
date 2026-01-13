package hva.Habitats;

import java.io.Serializable;
import hva.trees.*;
import hva.Animal.*;

import java.util.Map;
import java.util.TreeMap;

/**
 * Represents a habitat containing various species and trees.
 * Implements Serializable to allow habitat objects to be serialized.
 */
public class Habitat implements Serializable{  
    private String _id;
    private String _name;
    private int _population;
    private int _area;
    private int treesIn;
    private Map<String, String> _species = new TreeMap<>();
    private Map<String, Tree> _trees = new TreeMap<>(); 
    private Map<String, Animal> _animals = new TreeMap<>(); 


    /**
     * Constructs a Habitat object with specified properties.
     *
     * @param id    the unique identifier for the habitat
     * @param name  the name of the habitat
     * @param area  the area of the habitat
     */
    public Habitat(String id, String name, int area) {
        _id = id;
        _name = name;
        _area = area;
    }

    /**
     * Returns the unique identifier of the habitat.
     *
     * @return the habitat ID
     */
    public String getId() {
        return _id;
    }

    /**
     * Returns the name of the habitat.
     *
     * @return the habitat name
     */
    public String getName() {
        return _name;
    }

    /**
     * Returns the population of species in the habitat.
     *
     * @return the population of the habitat
     */
    public int getPopulation() {
        return _population;
    }

    /**
     * Returns the area of the habitat.
     *
     * @return the area of the habitat
     */
    public int getArea() {
        return _area;
    }

    /**
     * Returns the number of trees in the habitat.
     *
     * @return the count of trees in the habitat
     */
    public int getTreesIn() {
        return treesIn;
    }

    /**
     * Sets the area of the habitat to the specified value.
     *
     * @param area the new area value for the habitat
     */
    public void setArea(int area) {
        _area = area;
    }

    /**
     * Resets the number of trees in the habitat to zero.
     */
    public void setTreesIn() {
        treesIn = 0;
    }
    
    /**
     * Increments the count of trees in the habitat by one.
     */
    public void addTree() {
        treesIn++;
    }


    public void removeTree() {
        treesIn--;
    }


    public Map<String, String> getSpecies() {
        return _species;
    }

    /**
     * Returns a map of trees in the habitat.
     *
     * @return a map containing tree IDs and their corresponding Tree objects
     */
    public Map<String, Tree> getTrees() {
        return _trees;
    }

    public Map<String, Animal> getAnimals() {
        return _animals;
    }
}