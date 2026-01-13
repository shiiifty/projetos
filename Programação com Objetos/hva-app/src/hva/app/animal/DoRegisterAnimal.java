package hva.app.animal;

import pt.tecnico.uilib.forms.Form;

import hva.Hotel;
import hva.app.exceptions.DuplicateAnimalKeyException;
import hva.app.exceptions.UnknownHabitatKeyException;
import pt.tecnico.uilib.forms.Form;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;



class DoRegisterAnimal extends Command<Hotel> {

    DoRegisterAnimal(Hotel receiver) {
        super(Label.REGISTER_ANIMAL, receiver);
    }

    @Override
    protected final void execute() throws CommandException {
        Form request = new Form();
        try {
            request.addStringField("id", Prompt.animalKey());
            request.addStringField("name", Prompt.animalName());
            request.addStringField("specieId", Prompt.speciesKey());
            request.addStringField("habitatId", hva.app.habitat.Prompt.habitatKey());
            request.parse();
            _receiver.registerAnimal(request.stringField("id"), request.stringField("name"), request.stringField("specieId"), request.stringField("habitatId"));
        } catch(hva.exceptions.UnknownHabitatKeyException e) {throw new UnknownHabitatKeyException(request.stringField("habitatId"));}
        catch (hva.exceptions.DuplicateAnimalKeyException e) {throw new DuplicateAnimalKeyException(request.stringField("id"));}
    }
}