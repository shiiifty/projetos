package hva.Employees;

import java.util.Map;
import java.util.List;
import java.util.ArrayList;
import java.util.TreeMap;
import hva.Habitats.Habitat;

/**
 * Represents a veterinarian (VET) employee with specific responsibilities.
 * Inherits from the Employee class.
 */
public class VET extends Employee {

    private Map<String, String> _species = new TreeMap<>();
    private List<String> _vetVaccinations = new ArrayList<>();
    private String _job = "VET";

    public VET(String id, String name, List<String> responsibilities) {
        super(id, name, responsibilities);

    }

    /**
     * Constructs a VET object with specified ID and name.
     *
     * @param id   the unique identifier for the veterinarian
     * @param name the name of the veterinarian
     */
    public VET(String id, String name) {
        super(id, name); // Calls the constructor of the Employee class
    }

    /**
     * Returns the job title of the veterinarian.
     *
     * @return the job title of the employee
     */
    @Override
    public String getJob() {
        return _job; // Returns the job title
    }

    @Override
    public List<String> getVetVaccinations() {
        return _vetVaccinations;
    }
}