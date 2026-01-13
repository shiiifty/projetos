package hva.app.search;

import pt.tecnico.uilib.forms.Form;

import hva.Hotel;
import hva.app.animal.Prompt;
import hva.app.exceptions.UnknownAnimalKeyException;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;
//FIXME import other classes if needed

class DoShowMedicalActsOnAnimal extends Command<Hotel> {

    DoShowMedicalActsOnAnimal(Hotel receiver) {
        super(Label.MEDICAL_ACTS_ON_ANIMAL, receiver);
    }

    @Override
    protected void execute() throws CommandException {
        Form request = new Form();
        try {
            request.addStringField("animalId", Prompt.animalKey());
            request.parse();
            _display.popup(_receiver.getHealthHistory(request.stringField("animalId")));
        } catch (hva.exceptions.UnknownAnimalKeyException e) {throw new UnknownAnimalKeyException(request.stringField("animalId"));}
    }
}
