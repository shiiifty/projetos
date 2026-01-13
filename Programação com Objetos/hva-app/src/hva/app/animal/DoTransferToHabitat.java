package hva.app.animal;

import pt.tecnico.uilib.forms.Form;

import hva.Hotel;
import hva.app.exceptions.UnknownAnimalKeyException;
import hva.app.exceptions.UnknownHabitatKeyException;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;

class DoTransferToHabitat extends Command<Hotel> {

    DoTransferToHabitat(Hotel hotel) {
        super(Label.TRANSFER_ANIMAL_TO_HABITAT, hotel);
        
    }

    @Override
    protected final void execute() throws CommandException {
        Form request = new Form();
        try {
            request.addStringField("id", hva.app.animal.Prompt.animalKey());
            request.addStringField("habitatId", hva.app.habitat.Prompt.habitatKey());
            request.parse();
            _receiver.transferToHabitat(request.stringField("habitatId"), request.stringField("id")); 
        } catch (hva.exceptions.UnknownHabitatKeyException e) {throw new UnknownHabitatKeyException(request.stringField("habitatId"));}
        catch (hva.exceptions.UnknownAnimalKeyException e) {throw new UnknownAnimalKeyException(request.stringField("id"));}
    } 
}