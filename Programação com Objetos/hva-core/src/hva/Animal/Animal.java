package hva.Animal;


import java.io.Serializable;
import java.util.List;
import java.util.ArrayList;
import java.util.Collections;

import hva.trees.Tree;


public class Animal implements Serializable{
    private String _id;
    private String _name;
    private String _specie;
    private int _satisfaction;
    private String _idHabitat;
    private List<String> _healthHistory = new ArrayList<>();
    private int numberVaccinations = 0;


    /**
     * Constructs an Animal object with specified ID, name, species, and habitat ID.
     *
     * @param id        the unique identifier for the animal
     * @param name      the name of the animal
     * @param specie    the species of the animal
     * @param habitatId the unique identifier of the habitat the animal belongs to
     */
    public Animal(String id, String name, String specie, String habitatId) {
        _id = id;
        _name = name;
        _specie = specie;
        _idHabitat = habitatId;
    }

    /**
     * Returns the unique identifier of the animal.
     *
     * @return the animal ID
     */
    public String getId() {
        return _id;
    } 
    
    /**
     * Returns the name of the animal.
     *
     * @return the animal name
     */
    public String getName() {
        return _name;
    }

    /**
     * Returns the species of the animal.
     *
     * @return the animal species
     */
    public String getSpecie() {
        return _specie;
    }

    /**
     * Returns the satisfaction level of the animal.
     *
     * @return the animal's satisfaction level
     */
    public int getSatisfaction() {
        return _satisfaction;
    }

    /**
     * Returns the unique identifier of the habitat the animal belongs to.
     *
     * @return the habitat ID
     */
    public String getHabitatId() {
        return _idHabitat;
    }


    public List<String> getHealthHistory() {
        return _healthHistory;
    }


    public void setHabitatId(String habitatId) {
        _idHabitat = habitatId;
    }

    public void addVaccination() {
        numberVaccinations++;
    }

    public int getNumberVaccinations() {
        return numberVaccinations;
    }
}