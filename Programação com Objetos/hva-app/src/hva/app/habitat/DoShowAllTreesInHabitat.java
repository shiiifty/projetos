package hva.app.habitat;

import pt.tecnico.uilib.forms.Form;

import hva.Hotel;
import hva.app.exceptions.UnknownHabitatKeyException;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;

class DoShowAllTreesInHabitat extends Command<Hotel> {

    DoShowAllTreesInHabitat(Hotel receiver) {
        super(Label.SHOW_TREES_IN_HABITAT, receiver);
    }

    @Override
    protected void execute() throws CommandException {
        Form request = new Form();
        try {
            request.addStringField("habitatId", Prompt.habitatKey());
            request.parse();
           String[] outputs = _receiver.getAllTreesInHabitat(request.stringField("habitatId"));
            for (String output: outputs) _display.popup(output);
        } catch (hva.exceptions.UnknownHabitatKeyException e) {throw new UnknownHabitatKeyException(request.stringField("habitatId"));}
    }
}