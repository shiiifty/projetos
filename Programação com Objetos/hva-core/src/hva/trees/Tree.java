package hva.trees;

import java.io.Serializable;

/**
 * Represents a tree with its properties and methods to calculate biological cycles and effort.
 * Implements Serializable to allow tree objects to be serialized.
 */
public class Tree implements Serializable{
    private String _id;
    private String _name;
    private String _type;
    private int _age;
    private int _difficulty;
    private int _seasonCounter = 0;

    /**
     * Constructs a Tree object with specified properties.
     *
     * @param id         the unique identifier for the tree
     * @param name       the name of the tree
     * @param type       the type of the tree
     * @param age        the age of the tree
     * @param difficulty  the difficulty level of caring for the tree
     */
    public Tree(String id, String name, String type, int age, int difficulty) {
        _id = id;
        _name = name;
        _type = type;
        _age = age;
        _difficulty = difficulty;
    }

    /**
     * Returns the unique identifier of the tree.
     *
     * @return the tree ID
     */
    public String getId() {
        return _id;
    }

    /**
     * Returns the name of the tree.
     *
     * @return the tree name
     */
    public String getName() {
        return _name;
    }

    /**
     * Returns the type of the tree.
     *
     * @return the tree type
     */
    public String getType() {
        return _type;
    }

    /**
     * Returns the age of the tree.
     *
     * @return the age of the tree
     */
    public int getAge() {
        return _age;
    }

    /**
     * Returns the difficulty level associated with the tree.
     *
     * @return the difficulty level
     */
    public int getDifficulty() {
        return _difficulty;
    }


    public int getSeasonCounter() {
        return _seasonCounter;
    }


    public String getBiologicalCicle(String type, String season) {
        switch (season) {
            case "INVERNO":
                if (type.equals("CADUCA")) return "SEMFOLHAS";
                else if (type.equals("PERENE")) return "LARGARFOLHAS";
            case "PRIMAVERA":
                return "GERARFOLHAS";
            case "VERÃO":
                return "COMFOLHAS";
            case "OUTONO":
                if (type.equals("CADUCA")) return "LARGARFOLHAS";
                else if (type.equals("PERENE")) return "SEMFOLHAS";       
            default:
                return ".";
        }
    }

    /**
     * Calculates the effort required to maintain the tree based on its age, type, and difficulty.
     *
     * @param season the current season to evaluate the effort
     * @return the calculated effort based on tree type, age, and difficulty
     */
    public double calculateEffort(String season) {
        int effort = 0;
        if (_type == "CADUCA") {
            effort = TreeType.CADUCA.getEffort(season);
        }
        else if (_type == "PERENE") {
            effort = TreeType.PERENE.getEffort(season);
        }
        return (effort * Math.log(_age + 1) * _difficulty);
    }

    public void addSeasonCounter() {
        _seasonCounter++;
    }

    public void addAge() {
        _age++;
    }
}