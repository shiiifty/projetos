package hva.Vaccines;

import java.io.Serializable;
import java.util.List;
import java.util.ArrayList;

/**
 * Represents a vaccine with its details and usage information.
 * Implements Serializable to allow vaccine objects to be serialized.
 */
public class Vaccine implements Serializable{
    private String _vaccineId;
    private String _vaccineName;
    private int _timesUsed;
    private List<String> _apllicableSpecies = new ArrayList<>();


    public Vaccine(String vaccineId, String vaccineName, List<String> applicableSpecies) {

        _vaccineId = vaccineId;
        _vaccineName = vaccineName;
        _apllicableSpecies = applicableSpecies;
    }

    /**
     * Constructs a Vaccine object with the specified ID and name.
     * Applicable species are not specified in this constructor.
     *
     * @param vaccineId      the unique identifier for the vaccine
     * @param vaccineName    the name of the vaccine
     */
    public Vaccine(String vaccineId, String vaccineName) {
        _vaccineId = vaccineId;
        _vaccineName = vaccineName;
    }

    /**
     * Returns the unique identifier for the vaccine.
     *
     * @return the vaccine ID
     */
    public String getVaccineId() {
        return _vaccineId;
    }

    /**
     * Returns the name of the vaccine.
     *
     * @return the vaccine name
     */
    public String getVaccineName() {
        return _vaccineName;
    }

    /**
     * Returns the number of times the vaccine has been used.
     *
     * @return the number of times the vaccine has been used
     */
    public int getTimesUsed() {
        return _timesUsed;
    }


    public List<String> getApllicableSpecies() {

        return _apllicableSpecies;
    }
    
    /**
     * Resets the usage count of the vaccine to zero.
     */
    public void setTimesUsed() {
        _timesUsed = 0;
    }

    /**
     * Increments the usage count of the vaccine by one.
     */
    public void oneMoreTimeUsed() {
        _timesUsed++;
    }
}