package hva;

import java.io.ObjectOutputStream;
import java.io.FileOutputStream;
import java.io.BufferedOutputStream;
import java.io.FileNotFoundException;
import java.io.ObjectInputStream;
import java.io.FileInputStream;
import java.io.BufferedInputStream;



import hva.exceptions.MissingFileAssociationException;
import hva.exceptions.UnavailableFileException;
import hva.exceptions.UnknownVaccineKeyException;
import hva.exceptions.UnknownVeterinarianKeyException;
import hva.exceptions.DuplicateAnimalKeyException;
import hva.exceptions.DuplicateEmployeeKeyException;
import hva.exceptions.DuplicateHabitatKeyException;
import hva.exceptions.DuplicateTreeKeyException;
import hva.exceptions.DuplicateVaccineKeyException;
import hva.exceptions.ImportFileException;
import hva.exceptions.NoResponsibilityException;
import hva.exceptions.VeterinarianNotAuthorizedException;
import hva.exceptions.UnknownAnimalKeyException;
import hva.exceptions.UnknownEmployeeKeyException;
import hva.exceptions.UnknownHabitatKeyException;
import hva.exceptions.UnknownSpeciesKeyException;

import java.io.IOException;


/**
 * Class that represents the hotel application.
 */
public class HotelManager {

    /** This is the current hotel. */
    private Hotel _hotel = new Hotel();

    /** Filename for saving and loading hotel data. */
    private String _filename = "";

    /**
     * Advances the season in the hotel, triggering any seasonal changes.
     */
    public void advanceSeason() {
        this._hotel.advanceSeason();
    }


    public String seasonNumber() {
        return this._hotel.seasonNumber();
    }


    public boolean changed() {
        return this._hotel.hasChanged();
    }


    public void registerAnimal(String id, String name, String specie, String habitatId) throws UnknownHabitatKeyException, DuplicateAnimalKeyException{

        this._hotel.registerAnimal(id, name, specie, habitatId);
    }

    /**
     * Retrieves and displays all animals in the hotel.
     */
    public void getAllAnimals() {
        this._hotel.getAllAnimals();
    }

    /**
     * Displays the satisfaction of a specific animal.
     *
     * @param id the unique identifier of the animal.
     * @throws UnknownAnimalKeyException if the animal with the given ID does not exist.
     */
    public void showSatisfactionOfAnimal(String id) throws UnknownAnimalKeyException{
        this._hotel.showSatisfactionOfAnimal(id);
    }

    /**
     * Transfers an animal to a different habitat.
     *
     * @param habitatId the ID of the destination habitat.
     * @param id        the unique identifier of the animal.
     * @throws UnknownAnimalKeyException  if the animal with the given ID does not exist.
     * @throws UnknownHabitatKeyException if the habitat with the given ID does not exist.
     */
    public void transferToHabitat(String habitatId, String id) throws UnknownAnimalKeyException, UnknownHabitatKeyException {
        this._hotel.transferToHabitat(habitatId, id);
    }


    public String addTreeToHabitat(String habitatId, String id, String name, int age, int difficulty, String type) throws DuplicateTreeKeyException, UnknownHabitatKeyException {
        return this._hotel.addTreeToHabitat(habitatId, id, name, age, difficulty, type);
    }

    /**
     * Changes the area of a specific habitat.
     *
     * @param habitatId the ID of the habitat.
     * @param area      the new area of the habitat.
     * @throws UnknownHabitatKeyException if the habitat with the given ID does not exist.
     */
    public void changeHabitatArea(String habitatId, int area) throws UnknownHabitatKeyException {
        this._hotel.changeHabitatArea(habitatId, area);
    }


    public void changeHabitatInfluence(String habitatId, String specieId, String influence) throws UnknownHabitatKeyException, UnknownSpeciesKeyException{

        this._hotel.changeHabitatInfluence(habitatId, specieId, influence);
    }

    /**
     * Registers a new habitat in the hotel.
     *
     * @param habitatId the unique identifier of the habitat.
     * @param name      the name of the habitat.
     * @param area      the area of the habitat.
     * @throws DuplicateHabitatKeyException if a habitat with the given ID already exists.
     */
    public void registerHabitat(String habitatId, String name, int area) throws DuplicateHabitatKeyException {
        this._hotel.registerHabitat(habitatId, name, area);
    }

    public int getHabitats() {
        return this._hotel.getHabitats();
    }

    public void noVaccineAnimals() {
        this._hotel.noVaccineAnimals();
    }

    public void noTreesHabitat() {
        this._hotel.noTreesHabitat();
    }


    public void registerTree(String treeId, String treeName, int treeAge, int treeDifficulty, String type) throws DuplicateTreeKeyException {
        this._hotel.registerTree(treeId, treeName, treeAge, treeDifficulty, type);
    }


    public void getAllHabitats() {
        this._hotel.getAllHabitats();
    }

    /**
     * Retrieves and displays all trees in a specific habitat.
     *
     * @param habitatId the ID of the habitat.
     * @throws UnknownHabitatKeyException if the habitat with the given ID does not exist.
     */
    public void getAllTreesInHabitat(String habitatId) throws UnknownHabitatKeyException {
        this._hotel.getAllTreesInHabitat(habitatId);
    }

    public void addResponsibility(String employeeId, String responsibilityId) throws UnknownEmployeeKeyException{
        this._hotel.addResponsibility(employeeId, responsibilityId);
    }

    public void removeResponsibility(String employeeId, String responsibilityId) throws UnknownEmployeeKeyException, NoResponsibilityException{
        this._hotel.removeResponsibility(employeeId, responsibilityId);
    }

    /**
     * Registers a new employee in the hotel.
     *
     * @param type            the type of employee (e.g., caretaker, manager).
     * @param id              the unique identifier of the employee.
     * @param name            the name of the employee.
     * @param responsibilities the responsibilities assigned to the employee.
     * @throws DuplicateEmployeeKeyException if an employee with the given ID already exists.
     */
    public void registerEmployee(String type, String id, String name, String responsibilities) throws DuplicateEmployeeKeyException{
        this._hotel.registerEmployee(type, id, name, responsibilities);
    }

    /**
     * Retrieves and displays all employees in the hotel.
     */
    public void getAllEmployees() {
        this._hotel.getAllEmployees();
    }

    public String giveSpecie(String VaccineApplicable) {
        return this._hotel.giveSpecie(VaccineApplicable);
    }

    public void registerVaccine(String vaccineId, String vaccineName, String vaccineApplicable) throws DuplicateVaccineKeyException, UnknownSpeciesKeyException{

        this._hotel.registerVaccine(vaccineId, vaccineName, vaccineApplicable);
    }

    /**
     * Retrieves and displays all vaccines in the hotel.
     */
    public void getAllVccines() {
        this._hotel.getAllVaccines();
    }

    public String getSpecie(String animalId) {
        return this._hotel.getSpecie(animalId);
    }

    public void vaccinateAnimal(String vaccineId, String veterinarianId, String animalId) throws UnknownAnimalKeyException, UnknownVeterinarianKeyException, UnknownVaccineKeyException, VeterinarianNotAuthorizedException{
        this._hotel.vaccinateAnimal(vaccineId, veterinarianId, animalId);
    }
    

    public void applicable(String vaccineId, String animalId) {
        this._hotel.applicable(vaccineId, animalId);
    }

    public void getVaccionations() {
        this._hotel.getVaccionations();
    }

    public void getAnimalsHabitat(String habitatId) {
        this._hotel.getAnimalsHabitat(habitatId);
    }

    public void getHealthHistory(String animalId) throws UnknownAnimalKeyException{
        this._hotel.getHealthHistory(animalId);
    }

    public void getVeterinarianMedicalActs(String employeeId) throws UnknownVeterinarianKeyException {
        this._hotel.getVeterinarianMedicalActs(employeeId);
    }

    public void getBadVaccinations() {
        this._hotel.getBadVaccinations();
    }

    // FIXME maybe add more fields if needed

    
    /**
     * Saves the serialized application's state into the file associated with the current hotel.
     *
     * @throws MissingFileAssociationException if the current hotel does not have an associated file.
     * @throws IOException if there is an error while saving the state of the hotel.
     */
    public void save() throws MissingFileAssociationException, IOException {
        if (_filename == null || _filename.equals(""))
            throw new MissingFileAssociationException();
        try (ObjectOutputStream oos = new ObjectOutputStream(new BufferedOutputStream(new FileOutputStream(_filename)))) {
            oos.writeObject(_hotel);
            _hotel.setChanged(false);
        }
    }

    /**
     * Saves the serialized application's state to a specific file.
     *
     * @param filename the name of the file to save the hotel's state.
     * @throws FileNotFoundException if the file cannot be created or opened.
     * @throws MissingFileAssociationException if the file association is missing.
     * @throws IOException if there is an error while saving the state of the hotel.
     */
    public void saveAs(String filename) throws FileNotFoundException, MissingFileAssociationException, IOException {
        _filename = filename;
        save();
    }

    /**
     * Loads the serialized application's state from a file.
     *
     * @param filename the name of the file containing the serialized hotel's state.
     * @throws UnavailableFileException if the file does not exist or an error occurs while loading the file.
     */
    public void load(String filename) throws UnavailableFileException{
        _filename = filename;
         try (ObjectInputStream ois = new ObjectInputStream(new BufferedInputStream(new FileInputStream(filename)))) {
            _hotel = (Hotel) ois.readObject();
            _hotel.setChanged(false);
        } catch (IOException | ClassNotFoundException e) {
            throw new UnavailableFileException(filename);
        }
    }

    /**
     * Read text input file.
     *
     * @param filename name of the text input file
     * @throws ImportFileException
     */
    public void importFile(String filename) throws ImportFileException {
        _hotel.importFile(filename);
    }

    /**
     * Resets the hotel to its initial state by creating a new instance of the Hotel class.
     * The filename associated with the hotel is also set to null, clearing any previous association.
     */
    public void reset() {
        _hotel = new Hotel();
        _filename = null;
    }

    /**
     * Returns the current hotel instance.
     *
     * @return the current {@link Hotel} object.
     */
    public Hotel getHotel() { return _hotel;}
}