package hva.app.search;

import pt.tecnico.uilib.forms.Form;

import hva.Hotel;
import hva.app.habitat.Prompt;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;
//FIXME import other classes if needed

class DoShowAnimalsInHabitat extends Command<Hotel> {

    DoShowAnimalsInHabitat(Hotel receiver) {
        super(Label.ANIMALS_IN_HABITAT, receiver);
    }


    //este e o comentario novo sera que vai dar para dar push?
    @Override
    protected void execute() throws CommandException {
        Form request = new Form();
        request.addStringField("habitatId", Prompt.habitatKey());
        request.parse();
        String[] outputs = _receiver.getAnimalsHabitat(request.stringField("habitatId"));
        for (String output: outputs) _display.popup(output);
    }
}
