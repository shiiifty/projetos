package hva.Employees;

import java.io.Serializable;
import java.util.List;
import java.util.ArrayList;


public abstract class Employee implements Serializable{
    private String _id;
    private String _name;
    private List<String> _responsibilities = new ArrayList<>();

    public Employee(String id, String name, List<String> responsibilities) {

        _id = id;
        _name = name;
        _responsibilities = responsibilities;
    }

    /**
     * Constructs an Employee object with specified ID and name.
     *
     * @param id   the unique identifier for the employee
     * @param name the name of the employee
     */
    public Employee(String id, String name) {
        _id = id;
        _name = name;
    }

    /**
     * Returns the unique identifier of the employee.
     *
     * @return the employee ID
     */
    public String getId() {
        return _id;
    }

    /**
     * Returns the name of the employee.
     *
     * @return the employee name
     */
    public String getName() {
        return _name;
    }


    public List<String> getResponsibilities() {

        return _responsibilities;
    }

    /**
     * Abstract method to get the job title of the employee.
     * This method must be implemented by subclasses.
     *
     * @return the job title of the employee
     */
    public abstract String getJob();
    public abstract List<String> getVetVaccinations();
}