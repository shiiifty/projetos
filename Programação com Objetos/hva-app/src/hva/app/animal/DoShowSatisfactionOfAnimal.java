package hva.app.animal;

import pt.tecnico.uilib.forms.Form;

import hva.Hotel;
import hva.app.exceptions.UnknownAnimalKeyException;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;

class DoShowSatisfactionOfAnimal extends Command<Hotel> {

    DoShowSatisfactionOfAnimal(Hotel receiver) {
        super(Label.SHOW_SATISFACTION_OF_ANIMAL, receiver);
    }

    @Override
    protected final void execute() throws CommandException {
        Form request = new Form();
        try {
            request.addStringField("animalId", Prompt.animalKey());
            request.parse();
            _display.popup(_receiver.showSatisfactionOfAnimal(request.stringField("animalId")));
        } catch (hva.exceptions.UnknownAnimalKeyException e) {throw new UnknownAnimalKeyException(request.stringField("animalId"));}
    }
}