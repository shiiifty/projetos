package hva.app.habitat;

import pt.tecnico.uilib.forms.Form;

import hva.Hotel;
import hva.app.exceptions.UnknownHabitatKeyException;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;

class DoChangeHabitatArea extends Command<Hotel> {

    DoChangeHabitatArea(Hotel receiver) {
        super(Label.CHANGE_HABITAT_AREA, receiver);
    }

    @Override
    protected void execute() throws CommandException {
        Form request = new Form();
        try {
            request.addStringField("habitatId", Prompt.habitatKey());
            request.addIntegerField("area", Prompt.habitatArea());
            request.parse();
            _receiver.changeHabitatArea(request.stringField("habitatId"), request.integerField("area"));
        } catch (hva.exceptions.UnknownHabitatKeyException e) {throw new UnknownHabitatKeyException(request.stringField("habitatId"));}
    }
}