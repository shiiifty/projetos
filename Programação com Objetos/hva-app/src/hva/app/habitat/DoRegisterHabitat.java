package hva.app.habitat;

import pt.tecnico.uilib.forms.Form;

import hva.Hotel;
import hva.app.exceptions.DuplicateHabitatKeyException;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;

class DoRegisterHabitat extends Command<Hotel> {

    DoRegisterHabitat(Hotel receiver) {
        super(Label.REGISTER_HABITAT, receiver);
        
    }

    @Override
    protected void execute() throws CommandException {
        Form request = new Form();
        try {
            request.addStringField("id", Prompt.habitatKey());
            request.addStringField("name", Prompt.habitatName());
            request.addStringField("area", Prompt.habitatArea());
            request.parse();
            int area = Integer.parseInt(request.stringField("area"));
            _receiver.registerHabitat(request.stringField("id"), request.stringField("name"), area);
        } catch (hva.exceptions.DuplicateHabitatKeyException e) {throw new DuplicateHabitatKeyException(request.stringField("id"));}
    }
}