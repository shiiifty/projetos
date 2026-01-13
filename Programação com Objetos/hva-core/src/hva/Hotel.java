package hva;

import java.io.Serial;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;



import java.io.BufferedReader;
import java.io.FileReader;

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
import hva.Animal.Animal;
import hva.Animal.Specie;
import hva.Habitats.Habitat;
import hva.Vaccines.Vaccine;
import hva.trees.Tree;
import hva.Employees.Employee;
import hva.Employees.TRT;
import hva.Employees.VET;
import hva.exceptions.UnknownAnimalKeyException;
import hva.exceptions.UnknownEmployeeKeyException;
import hva.exceptions.UnknownHabitatKeyException;
import hva.exceptions.UnknownSpeciesKeyException;

import java.io.IOException;
//FIXME import project classes

public class Hotel implements Serializable {

    @Serial
    private static final long serialVersionUID = 202407081733L;


    private Map<String, Animal> _animals = new TreeMap<>(new CaseInsensitiveComparator());
    private Map<String, Habitat> _habitats = new TreeMap<>(new CaseInsensitiveComparator());
    private Map<String, Employee> _employees = new TreeMap<>(new CaseInsensitiveComparator());
    private Map<String, Vaccine> _vaccines = new TreeMap<>(new CaseInsensitiveComparator());
    private Map<String, Specie> _species = new TreeMap<>(new CaseInsensitiveComparator());
    private Map<String, Tree> _trees = new TreeMap<>(new CaseInsensitiveComparator());
    private List<String> _vaccinations = new ArrayList<>();
    private List<String> _badVaccinations = new ArrayList<>();
    private boolean _changed = false;
    private String _season = "PRIMAVERA";
    private int seasonCounter = 4;


    private static final String[] SEASONS = {"PRIMAVERA", "VERÃO", "OUTONO", "INVERNO"};

    /**
     * Returns the current season.
     *
     * @return the current season as a String.
     */
    public String getSeason() {
        return _season;
    }

    /**
     * Advances the current season in a cycle:
     * PRIMAVERA -> VERAO -> OUTONO -> INVERNO -> PRIMAVERA.
     */
    public void advanceSeason() {

        seasonCounter++;
        _season = SEASONS[seasonCounter % 4];
        for (Map.Entry<String, Habitat> entry : _habitats.entrySet()) {
            for (Map.Entry<String, Tree> entry2 : entry.getValue().getTrees().entrySet()) {
                entry2.getValue().addSeasonCounter();
                if (entry2.getValue().getSeasonCounter() % 4 == 0) entry2.getValue().addAge();
            }
        }
    }

    public String seasonNumber() {
        switch (_season) {
            case "PRIMAVERA":
                return "0";
            case "VERÃO":
                return "1";
            case "OUTONO":
                return "2";
            case "INVERNO":
                return "3";
            default:
                return "0";
        }
    }       

    /**
     * Marks that the hotel's data has been changed.
     */
    public void changed() {
        setChanged(true);
    }

    /**
     * Checks if the hotel's data has changed.
     *
     * @return true if data has changed, false otherwise.
     */
    public boolean hasChanged() {
        return _changed;
    }

    /**
     * Sets the change status of the hotel's data.
     *
     * @param b true to indicate data has changed, false otherwise.
     */
    public void setChanged(boolean b) {
        _changed = b;
    }

    /**
     * Registers a new animal in the hotel.
     *
     * @param id        the animal's unique identifier.
     * @param name      the animal's name.
     * @param specieId  the species ID of the animal.
     * @param habitatId the habitat ID where the animal is located.
     */

    public void registerAnimal(String id, String name, String specieId, String habitatId) throws UnknownHabitatKeyException, DuplicateAnimalKeyException{
        if (_habitats.get(habitatId) == null) throw new UnknownHabitatKeyException(habitatId);
        if (_animals.get(id) != null) throw new DuplicateAnimalKeyException(id);
        Animal a = new Animal(id, name, specieId, habitatId);
        _animals.put(id, a);
        _habitats.get(habitatId).getAnimals().put(id, a);
    }

    /**
     * Registers a new species in the hotel.
     *
     * @param specieId   the species unique identifier.
     * @param specieName the species name.
     */
    public void registerSpecie(String specieId, String specieName) {
        Specie newSpecie = new Specie(specieId, specieName);
        _species.put(specieId, newSpecie);
    }

    /**
     * Retrieves information about all animals registered in the hotel.
     *
     * @return an array of strings with each animal's details.
     */
    public String[] getAllAnimals() {
    String[] allOutputs = new String[_animals.size()];
    int index = 0;
    if (_animals != null) {
        for (Map.Entry<String, Animal> entry : _animals.entrySet()) {
        String output = "ANIMAL|" + entry.getKey() + "|" + entry.getValue().getName() + "|" + entry.getValue().getSpecie();

        if (entry.getValue().getHealthHistory().size() != 0) {
            List<String> healthHistory = entry.getValue().getHealthHistory();
            output += "|";
            for (int i = 0; i < healthHistory.size(); i++) {
                output += healthHistory.get(i);
                if (i < healthHistory.size() - 1) {

                output += ",";
                } else {
                output += "|";
                }
            }
        } else {
            output += "|VOID|";
        }
        output += entry.getValue().getHabitatId();
        allOutputs[index] = output;
        index++;
        }
    }
    return allOutputs;
    }

    /**
     * Displays the satisfaction level of an animal.
     *
     * @param id the animal's unique identifier.
     * @throws UnknownAnimalKeyException if the animal ID is not found.
     */

    public int showSatisfactionOfAnimal(String id) throws UnknownAnimalKeyException {
        if (!_animals.containsKey(id)) throw new UnknownAnimalKeyException(id);
        int satisfaction =  _animals.get(id).getSatisfaction();
        return satisfaction;
    }

    /**
     * Transfers an animal to a new habitat.
     *
     * @param habitatId the habitat ID to transfer the animal to.
     * @param animalId  the animal's unique identifier.
     * @throws UnknownAnimalKeyException  if the animal ID is not found.
     * @throws UnknownHabitatKeyException if the habitat ID is not found.
     */
    public void transferToHabitat(String habitatId, String animalId) throws UnknownAnimalKeyException, UnknownHabitatKeyException {
        if (_habitats.get(habitatId) == null) throw new UnknownHabitatKeyException(habitatId);
        else if(_animals.get(animalId) == null) throw new UnknownAnimalKeyException(animalId);
        _habitats.get(_animals.get(animalId).getHabitatId()).getAnimals().remove(animalId);
        _habitats.get(_animals.get(animalId).getHabitatId()).removeTree();
        _animals.get(animalId).setHabitatId(habitatId);
        _habitats.get(habitatId).getAnimals().put(animalId, _animals.get(animalId));
        _habitats.get(habitatId).addTree();
    }

    /**
     * Adds a tree to a specific habitat.
     *
     * @param habitatId  the habitat ID.
     * @param id         the tree's unique identifier.
     * @param name       the tree's name.
     * @param age        the tree's age.
     * @param difficulty the difficulty level of maintaining the tree.
     * @param type       the type of the tree.
     */
    public String addTreeToHabitat(String habitatId, String id, String name, int age, int difficulty, String type) throws DuplicateTreeKeyException, UnknownHabitatKeyException{
        if (_habitats.get(habitatId) == null) throw new UnknownHabitatKeyException(habitatId);
        if (_trees.get(id) != null) throw new DuplicateTreeKeyException(id);
        Tree tree = new Tree(id, name, type, age, difficulty);
        _habitats.get(habitatId).getTrees().put(name, tree);
        _habitats.get(habitatId).addTree();
        String output = "ÁRVORE|" + id + "|" + name + "|" + age + "|" +difficulty + "|" + type + "|" + tree.getBiologicalCicle(type, _season);
        return output;
    }

    /**
     * Changes the area of a habitat.
     *
     * @param habitatId the habitat's unique identifier.
     * @param area      the new area of the habitat.
     * @throws UnknownHabitatKeyException if the habitat ID is not found.
     */
    public void changeHabitatArea(String habitatId, int area) throws UnknownHabitatKeyException {
        if (_habitats.get(habitatId) == null) throw new UnknownHabitatKeyException(habitatId);
        _habitats.get(habitatId).setArea(area);
    }

    /**
     * Changes the influence of a species in a specific habitat.
     *
     * @param habitatId the habitat's unique identifier.
     * @param specieId  the species unique identifier.
     * @param influence the influence level to be set.
     */
    public void changeHabitatInfluence(String habitatId, String specieId, String influence) throws UnknownHabitatKeyException, UnknownSpeciesKeyException{
        if (_habitats.get(habitatId) == null) throw new UnknownHabitatKeyException(habitatId);
        if (_species.get(specieId) == null) throw new UnknownSpeciesKeyException(specieId);
        _habitats.get(habitatId).getSpecies().put(specieId, influence);
    }

    /**
     * Registers a new habitat in the hotel.
     *
     * @param habitatId the habitat's unique identifier.
     * @param name      the habitat's name.
     * @param area      the area of the habitat.
     */
    public void registerHabitat(String habitatId, String name, int area) throws DuplicateHabitatKeyException{
        if (_habitats.get(habitatId) != null) throw new DuplicateHabitatKeyException(habitatId);
        Habitat habitat = new Habitat(habitatId, name, area);
        habitat.setTreesIn();
        _habitats.put(habitatId, habitat);
    }

    public int getHabitats() {
        return _habitats.size();
    }

    public String[] noVaccineAnimals() {
        String[] allOutputs = new String[_animals.size()];
        int index = 0;
        for (Map.Entry<String, Animal> entry : _animals.entrySet()) {
            if (entry.getValue().getHealthHistory().size() == 0) {
                String output = "ANIMAL|" + entry.getKey() + "|" + entry.getValue().getName() + "|" + entry.getValue().getSpecie() + "|VOID|" + entry.getValue().getHabitatId();
                allOutputs[index] = output;
                index++;
            }
        }
        return allOutputs;
    }
    

    public String[] noTreesHabitat() {
        List<String> outputs = new ArrayList<>();  // Using ArrayList to handle dynamic size
        if (_habitats != null) {
            for (Map.Entry<String, Habitat> entry : _habitats.entrySet()) {
                if (entry.getValue().getTreesIn() == 0) {
                    String output = "HABITAT|" + entry.getKey() + "|" + entry.getValue().getName() + "|" + entry.getValue().getArea() + "|" + entry.getValue().getTreesIn();
                    outputs.add(output);
                }
            }
        }
        return outputs.toArray(new String[0]);
    }


    public void registerTree(String treeId, String treeName, int treeAge, int treeDifficulty, String type) throws DuplicateTreeKeyException{
        if (_trees.get(treeId) != null) throw new DuplicateTreeKeyException(treeId);
        Tree tree = new Tree(treeId, treeName, type, treeAge, treeDifficulty);
        _trees.put(treeId, tree);
    }

    /**
     * Retrieves information about all habitats registered in the hotel.
     *
     * @return an array of strings with each habitat's details.
     */
    public String[] getAllHabitats() {
        List<String> outputs = new ArrayList<>();  // Using ArrayList to handle dynamic size
        if (_habitats != null) {
            for (Map.Entry<String, Habitat> entry : _habitats.entrySet()) {
            // Add habitat details to the list
                String output = "HABITAT|" + entry.getKey() + "|" + entry.getValue().getName() + "|" + entry.getValue().getArea() + "|" + entry.getValue().getTreesIn();
                outputs.add(output);

                try {
                // Check if there are trees in the habitat
                String[] treesInHabitat = getAllTreesInHabitat(entry.getKey());
                    if (treesInHabitat.length != 0) {
                        for (String tree : treesInHabitat) {
                        outputs.add(tree);  // Add each tree to the list
                        }
                    }
                } catch (UnknownHabitatKeyException e) {e.getKey();}
            }
        }
        return outputs.toArray(new String[0]);  // Convert ArrayList to array and return
    }


    /**
     * Retrieves all trees in a specific habitat.
     *
     * @param habitatId the habitat's unique identifier.
     * @return an array of strings with each tree's details in the specified habitat.
     * @throws UnknownHabitatKeyException if the habitat ID is not found.
     */
    public String[] getAllTreesInHabitat(String habitatId) throws UnknownHabitatKeyException {
        if (_habitats.get(habitatId) == null) throw new UnknownHabitatKeyException(habitatId);
        Habitat habitat = _habitats.get(habitatId);
        String[] outputs = new String[habitat.getTrees().size()];
        int index = 0;
        if (habitat.getTrees() != null) {
            for (Map.Entry<String, Tree> entry : habitat.getTrees().entrySet()) {
                String output = "ÁRVORE|" + entry.getValue().getId() +
                "|" + entry.getValue().getName() +
                "|" + entry.getValue().getAge() +
                "|" + entry.getValue().getDifficulty() +
                "|" + entry.getValue().getType() +
                "|" + entry.getValue().getBiologicalCicle(entry.getValue().getType(), getSeason());
                outputs[index] = output;
                index++;
            }
        }
        return outputs;
    }

    public void addResponsibility(String employeeId, String responsibilityId) throws UnknownEmployeeKeyException{
        if (!_employees.containsKey(employeeId)) throw new UnknownEmployeeKeyException(employeeId);
        _employees.get(employeeId).getResponsibilities().add(responsibilityId);
    }

    public void removeResponsibility(String employeeId, String responsibilityId) throws UnknownEmployeeKeyException, NoResponsibilityException{
        if (!_employees.containsKey(employeeId)) throw new UnknownEmployeeKeyException(employeeId);
        if (!_employees.get(employeeId).getResponsibilities().contains(responsibilityId)) throw new NoResponsibilityException(responsibilityId);
        _employees.get(employeeId).getResponsibilities().remove(responsibilityId);
    }


    /**
     * Registra um novo funcionário no sistema. O funcionário pode ser do tipo "TRATADOR" ou "VETERINÁRIO".
     *
     * @param type O tipo do funcionário (TRATADOR ou VETERINÁRIO).
     * @param id O ID do funcionário.
     * @param name O nome do funcionário.
     * @param responsibilities As responsabilidades do funcionário, separadas por vírgulas.
     */
    public void registerEmployee(String type, String id, String name, String responsibilities) throws DuplicateEmployeeKeyException {
        if (_employees.containsKey(id)) throw new DuplicateEmployeeKeyException(id);

        Employee _newEmployee = null;

        if (!responsibilities.equals("")) {
            List<String> responsibilitiesAux = new ArrayList<>();
            String[] splitResponsibilities = responsibilities.split(",");

            for (String responsibility : splitResponsibilities) {
                responsibilitiesAux.add(responsibility.trim());
            }

            if (type.equals("TRATADOR") || type.equals("TRT")) {
                _newEmployee = new TRT(id, name, responsibilitiesAux);
            } else if (type.equals("VETERINÁRIO") || type.equals("VET")) {
                _newEmployee = new VET(id, name, responsibilitiesAux);
            }
        } else {
            if (type.equals("TRATADOR") || type.equals("TRT")) {
                _newEmployee = new TRT(id, name);
            } else if (type.equals("VETERINÁRIO") || type.equals("VET")) {
                _newEmployee = new VET(id, name);
            }
        }

            _employees.put(id, _newEmployee);
    }




    /**
     * Retorna todos os funcionários registrados no sistema no formato de array de Strings.
     *
     * @return Um array de Strings contendo informações de todos os funcionários.
     */
    public String[] getAllEmployees() {
        String[] outputs = new String[_employees.size()];
        if (_employees != null) {
            int index = 0;
            for (Map.Entry<String, Employee> entry : _employees.entrySet()) {
                Employee employee = entry.getValue();
                if (employee != null) {
                    String output = employee.getJob() + "|" + entry.getKey() + "|" + employee.getName() + "|";
                    if (entry.getValue().getResponsibilities().size() != 0) {
                        for (String entry2 : entry.getValue().getResponsibilities()) {
                            output += entry2 + ",";
                        }
                    }
                    output = output.substring(0, output.length() - 1);
                    outputs[index] = output;
                    index++;
                }
            }
        }   
        return outputs;
    }

    public String giveSpecie(String vaccineApplicable) {
        String s = "";
        List<String> vaccineApplicableAux = Arrays.asList(vaccineApplicable.split(","));
        for (String vaccine: vaccineApplicableAux) {
            if (!_species.containsKey(vaccine)) s = vaccine;
        }
        return s;
    }

    /**
     * Registra uma nova vacina no sistema.
     *
     * @param vaccineId O ID da vacina.
     * @param name O nome da vacina.
     * @param vaccineApllicable As espécies às quais a vacina é aplicável, separadas por vírgulas.
     */
    public void registerVaccine(String vaccineId, String name, String vaccineApplicable) throws DuplicateVaccineKeyException, UnknownSpeciesKeyException {
        if (_vaccines.get(vaccineId) != null) throw new DuplicateVaccineKeyException(vaccineId);

        Vaccine _newVaccine;
        if (!vaccineApplicable.isEmpty()) {
            List<String> vaccineApplicableAux = Arrays.asList(vaccineApplicable.split(","));
            for (String vaccine: vaccineApplicableAux) {
                if (!_species.containsKey(vaccine)) throw new UnknownSpeciesKeyException(vaccine);
            }
            _newVaccine = new Vaccine(vaccineId, name, vaccineApplicableAux);
        } else {
            _newVaccine = new Vaccine(vaccineId, name);
        }

        _newVaccine.setTimesUsed();
        _vaccines.put(vaccineId, _newVaccine);
    }


    /**
     * Retorna todas as vacinas registradas no sistema no formato de array de Strings.
     *
     * @return Um array de Strings contendo informações de todas as vacinas.
     */
    public String[] getAllVaccines() {
        String[] outputs = new String[_vaccines.size()];
        int index = 0;
        for (Map.Entry<String, Vaccine> entry : _vaccines.entrySet()) {
            String output = "VACINA|" + entry.getKey() +
                "|" + entry.getValue().getVaccineName() +
                "|" + entry.getValue().getTimesUsed();
            if (entry.getValue().getApllicableSpecies().size() != 0) {
                output += "|";
                Collections.sort(entry.getValue().getApllicableSpecies(), String.CASE_INSENSITIVE_ORDER);

                // Concatenar os valores ordenados à string de saída
                output += String.join(",", entry.getValue().getApllicableSpecies());
            }
            if (output.endsWith(",")) {
                output = output.substring(0, output.length() - 1);
            }        
            outputs[index] = output;
            index++;
        }
        return outputs;
    }

    /**
     * Compara os tamanhos de dois nomes e retorna o tamanho do maior.
     *
     * @param name1 O primeiro nome a ser comparado.
     * @param name2 O segundo nome a ser comparado.
     * @return O tamanho do nome maior.
     */
    public int tamanho_nomes(String name1, String name2) {
        if (name1.length() > name2.length()) return name1.length();
        else return name2.length();
    }

    public int compareNames(String name1, String name2) {
        Set<Character> set1 = new HashSet<>();
        Set<Character> set2 = new HashSet<>();

        // Adiciona todos os caracteres de name1 no set1
        for (int i = 0; i < name1.length(); i++) {
            set1.add(name1.charAt(i));
        }

        // Adiciona todos os caracteres de name2 no set2
        for (int i = 0; i < name2.length(); i++) {
            set2.add(name2.charAt(i));
        }

        // Faz a interseção entre os dois sets (apenas os caracteres comuns ficam)
        set1.retainAll(set2);

        // Retorna o número de caracteres distintos em comum
        return set1.size();
    }


    public String dano(String vaccineId, String animalId, String veterinarianId) {
        int dano = 0;
        int danoAux;
        List<String> vaccines =  _vaccines.get(vaccineId).getApllicableSpecies();

        Animal animal = _animals.get(animalId);
        String specieName = _species.get(animal.getSpecie()).getSpecieName();
        if (vaccines.contains(animal.getSpecie())) return "NORMAL";
            for (int i = 0; i < vaccines.size(); i++) {
                String nameAux = _species.get(vaccines.get(i)).getSpecieName();
                danoAux = tamanho_nomes(specieName, nameAux) - compareNames(specieName, nameAux);
                if (danoAux > dano) dano = danoAux;
            }
        if (dano == 0) return "CONFUSÃO";
        else if (dano >= 1 && dano <= 4) return "ACIDENTE";
        else return "ERRO";
    }

    public String getSpecie(String animalId) {
        return _animals.get(animalId).getSpecie();
    }

    public void vaccinateAnimal(String vaccineId, String veterinarianId, String animalId) throws UnknownAnimalKeyException, UnknownVeterinarianKeyException, UnknownVaccineKeyException, VeterinarianNotAuthorizedException{
        if (_animals.get(animalId) == null) throw new UnknownAnimalKeyException(animalId);
        if (_employees.get(veterinarianId) == null || _employees.get(veterinarianId).getJob() == "TRT") throw new UnknownVeterinarianKeyException(veterinarianId);
        if (_vaccines.get(vaccineId) == null) throw new UnknownVaccineKeyException(vaccineId);
        if (!_employees.get(veterinarianId).getResponsibilities().contains(_animals.get(animalId).getSpecie())) throw new VeterinarianNotAuthorizedException(veterinarianId);
        
        Vaccine vaccine = _vaccines.get(vaccineId);
        Animal animal = _animals.get(animalId);
        String damage = dano(vaccineId, animalId, veterinarianId);
        if (damage.equals("ACIDENTE") || damage.equals("ERRO")) {
            _badVaccinations.add(vaccineId);
            _badVaccinations.add(veterinarianId);
            _badVaccinations.add(animal.getSpecie());
        }
        animal.getHealthHistory().add(damage);
        animal.addVaccination();
        vaccine.oneMoreTimeUsed();
        _employees.get(veterinarianId).getVetVaccinations().add(vaccineId);
        _employees.get(veterinarianId).getVetVaccinations().add(animal.getSpecie());
        _vaccinations.add(vaccineId);
        _vaccinations.add(veterinarianId);
        _vaccinations.add(animal.getSpecie());
    }

    public boolean applicable(String vaccineId, String animalId) {
        Animal animal = _animals.get(animalId);
        Vaccine vaccine = _vaccines.get(vaccineId);

        String specie = animal.getSpecie();
        List<String> applicableSpecies = vaccine.getApllicableSpecies();

        return applicableSpecies.contains(specie);
    }

    public String[] getVaccionations() {
        String[] outputs = new String[_vaccinations.size() / 3];
        int index = 0;
        for (int i = 0; i < _vaccinations.size(); i += 3) {
            String output = "REGISTO-VACINA" + 
            "|" + _vaccinations.get(i) + 
            "|" + _vaccinations.get(i + 1) + 
            "|" + _vaccinations.get(i + 2);

            outputs[index] = output;
            index++;
        }

        return outputs;
    }

    public String[] getAnimalsHabitat(String habitatId) {
        String[] outputs = new String[_habitats.get(habitatId).getAnimals().size()];
        int index = 0;
        
        if (_habitats.get(habitatId).getAnimals() != null) {
            for (Map.Entry<String, Animal> entry : _habitats.get(habitatId).getAnimals().entrySet()) {
                String output = "ANIMAL|" + entry.getKey() + "|" + entry.getValue().getName() + "|" + entry.getValue().getSpecie();
                if (entry.getValue().getHealthHistory().size() != 0) {
                    List<String> healthHistory = entry.getValue().getHealthHistory();
                    output += "|";
                    for (int i = 0; i < healthHistory.size(); i++) {
                        output += healthHistory.get(i);
                        if (i < healthHistory.size() - 1) {
                            output += ",";
                        } else {
                        output += "|";
                        }
                    }
                } else {
                    output += "|VOID|";
                }
                output += entry.getValue().getHabitatId();
                outputs[index] = output;
                index++;
            }
        }
        return outputs;
    }

    public String getHealthHistory(String animalId) throws UnknownAnimalKeyException{
        String output = "";
        if (_animals.get(animalId) == null) throw new UnknownAnimalKeyException(animalId);
        if (_animals.get(animalId).getHealthHistory() != null) {
            for (int i = 0; i < _animals.get(animalId).getHealthHistory().size(); i++) {
                output += _animals.get(animalId).getHealthHistory().get(i);
                if (i < _animals.get(animalId).getHealthHistory().size() - 1) {
                    output += ",";
                }
            }
        }
        return output;
    }

    public String[] getVeterinarianMedicalActs(String employeeId) throws UnknownVeterinarianKeyException{
        if (_employees.get(employeeId) == null) throw new UnknownVeterinarianKeyException(employeeId);
        if (!_employees.get(employeeId).getJob().equals("VET")) throw new UnknownVeterinarianKeyException(employeeId);
        
        String[] outputs = new String[_employees.get(employeeId).getVetVaccinations().size() / 2];
        int index = 0;
        for (int i = 0; i < _employees.get(employeeId).getVetVaccinations().size(); i += 2) {
            String output = "REGISTO-VACINA" + 
            "|" + _employees.get(employeeId).getVetVaccinations().get(i) + 
            "|" + employeeId + 
            "|" + _employees.get(employeeId).getVetVaccinations().get(i + 1);

            outputs[index] = output;
            index++;
        }
        return outputs;
    }

    public String[] getBadVaccinations() {
        String[] outputs = new String[_badVaccinations.size() / 3];
        int index = 0;
        for (int i = 0; i < _badVaccinations.size(); i += 3) {
            String output = "REGISTO-VACINA" + 
            "|" + _badVaccinations.get(i) + 
            "|" + _badVaccinations.get(i + 1) + 
            "|" + _badVaccinations.get(i + 2);

            outputs[index] = output;
            index++;
        }
        return outputs;
    }




    /**
     * Importa dados de um arquivo de texto para criar entidades de domínio no sistema.
     *
     * @param textFile O nome do arquivo de texto a ser importado.
     * @throws ImportFileException Se ocorrer um erro durante a leitura do arquivo.
     */
    void importFile(String textFile) throws ImportFileException {
        try (BufferedReader s = new BufferedReader(new FileReader(textFile))) {
            String line;
            while ((line = s.readLine()) != null) {
                importFromFields(line.split("\\|"));
            }

            this.changed();

            } catch (IOException e) {
            throw new ImportFileException(textFile, e);
        }
    }

    /**
     * Importa dados de um array de Strings e cria as entidades apropriadas no sistema com base no tipo de entrada.
     *
     * @param fields Um array de Strings contendo os dados a serem importados.
     */
    public void importFromFields(String[] fields) {
        switch (fields[0]) {
            case "ANIMAL" -> this.importAnimal(fields);
            case "HABITAT" -> this.importHabitat(fields);
            case "VETERINÁRIO", "TRATADOR" -> this.importEmployee(fields);
            case "VACINA" -> this.importVaccine(fields);
            case "ÁRVORE" -> this.importTree(fields);
            case "ESPÉCIE" -> this.importSpecie(fields);
        }
    }

    /**
     * Importa um animal com base nos campos fornecidos e registra no sistema.
     *
     * @param fields Um array de Strings contendo os dados do animal a ser importado.
     */
    public void importAnimal(String[] fields) {

        try {
            this.registerAnimal(fields[1], fields[2], fields[3], fields[4]);
        } catch (UnknownHabitatKeyException e) {e.getKey();}
        catch (DuplicateAnimalKeyException e) {e.getKey();}

    }

    /**
     * Importa um habitat com base nos campos fornecidos e registra no sistema.
     *
     * @param fields Um array de Strings contendo os dados do habitat a ser importado.
     */
    public void importHabitat(String[] fields) {

        try {
            this.registerHabitat(fields[1], fields[2], Integer.parseInt(fields[3]));
            if (fields.length == 5) {
                String[] trees = fields[4].split(",");
                for (int i = 0; i < trees.length; i++) {
                    _habitats.get(fields[1]).getTrees().put(trees[i], _trees.get(trees[i]));
                    _habitats.get(fields[1]).addTree();
                }
            }
        } catch (DuplicateHabitatKeyException e) {e.getKey();}
    }

    /**
     * Importa um funcionário com base nos campos fornecidos e registra no sistema.
     *
     * @param fields Um array de Strings contendo os dados do funcionário a ser importado.
     */
    public void importEmployee(String[] fields) {

        try {
            if (fields.length == 3) this.registerEmployee(fields[0], fields[1], fields[2], "");
            else this.registerEmployee(fields[0], fields[1], fields[2], fields[3]);
        } catch (DuplicateEmployeeKeyException e) {e.getKey();}

    }

    /**
     * Importa uma vacina com base nos campos fornecidos e registra no sistema.
     *
     * @param fields Um array de Strings contendo os dados da vacina a ser importada.
     */
    public void importVaccine(String[] fields) {

        try {
            if (fields.length == 3) this.registerVaccine(fields[1], fields[2], "");
            else this.registerVaccine(fields[1], fields[2], fields[3]);
        } catch (DuplicateVaccineKeyException e) {e.getKey();}
        catch (UnknownSpeciesKeyException e) {e.getKey();}

    }

    /**
     * Importa uma árvore com base nos campos fornecidos e registra no habitat apropriado no sistema.
     *
     * @param fields Um array de Strings contendo os dados da árvore a ser importada.
     */
    public void importTree(String[] fields) {

        try {
            this.registerTree(fields[1], fields[2], Integer.parseInt(fields[3]), Integer.parseInt(fields[4]), fields[5]);
        } catch (DuplicateTreeKeyException e) {e.getKey();}
    } 


    public void importSpecie(String[] fields) {
        this.registerSpecie(fields[1], fields[2]);
    }
}