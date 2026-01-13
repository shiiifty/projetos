package hva.app.vaccine;

import pt.tecnico.uilib.forms.Form;

import hva.Hotel;
import hva.app.exceptions.UnknownAnimalKeyException;
import hva.app.exceptions.UnknownVaccineKeyException;
import hva.app.exceptions.UnknownVeterinarianKeyException;
import hva.app.exceptions.VeterinarianNotAuthorizedException;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;
//FIXME import other classes if needed

class DoVaccinateAnimal extends Command<Hotel> {

    DoVaccinateAnimal(Hotel receiver) {
        super(Label.VACCINATE_ANIMAL, receiver);
    }

    @Override
    protected final void execute() throws CommandException {
        Form request = new Form();
        try{
            request.addStringField("vaccineId", Prompt.vaccineKey());
            request.addStringField("vetId", hva.app.vaccine.Prompt.veterinarianKey());
            request.addStringField("animalId", hva.app.animal.Prompt.animalKey());
            request.parse();
            _receiver.vaccinateAnimal(request.stringField("vaccineId"), request.stringField("vetId"), request.stringField("animalId"));
        } catch (hva.exceptions.UnknownAnimalKeyException e) {throw new UnknownAnimalKeyException(request.stringField("animalId"));}
        catch (hva.exceptions.UnknownVaccineKeyException e) {throw new UnknownVaccineKeyException(request.stringField("vaccineId"));}
        catch (hva.exceptions.UnknownVeterinarianKeyException e) {throw new UnknownVeterinarianKeyException(request.stringField("vetId"));}
        catch (hva.exceptions.VeterinarianNotAuthorizedException e) {throw new VeterinarianNotAuthorizedException(request.stringField("vetId"), _receiver.getSpecie(request.stringField("animalId")));}

        if (!_receiver.applicable(request.stringField("vaccineId"), request.stringField("animalId"))) {
            _display.popup(Message.wrongVaccine(request.stringField("vaccineId"), request.stringField("animalId")));
        }
    }
}