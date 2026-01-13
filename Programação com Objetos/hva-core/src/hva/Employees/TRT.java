package hva.Employees;

import hva.Habitats.Habitat;


import java.util.Map;
import java.util.List;
import java.util.TreeMap;


public class TRT extends Employee {
    private String _job = "TRT"; // Job title of the employee


    public TRT(String id, String name, List<String> responsibilities) {
        super(id, name, responsibilities);

    }

    /**
     * Constructs a TRT object with specified ID and name.
     *
     * @param id   the unique identifier for the TRT employee
     * @param name the name of the TRT employee
     */
    public TRT(String id, String name) {
        super(id, name); // Calls the constructor of the Employee class
    }

    /**
     * Returns the job title of the TRT employee.
     *
     * @return the job title of the employee
     */
    @Override
    public String getJob() {
        return _job; // Returns the job title
    }

    @Override
    public List<String> getVetVaccinations() {
        throw new UnsupportedOperationException("Tratador não possui vacinações.");
    }
}