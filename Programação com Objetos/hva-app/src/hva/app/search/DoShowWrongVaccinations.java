package hva.app.search;

import hva.Hotel;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;
//FIXME import other classes if needed

class DoShowWrongVaccinations extends Command<Hotel> {

    DoShowWrongVaccinations(Hotel receiver) {
        super(Label.WRONG_VACCINATIONS, receiver);
	//FIXME add command fields if needed
    }

    @Override
    protected void execute() throws CommandException {
        String[] outputs = _receiver.getBadVaccinations();
        for (String output: outputs) _display.popup(output);
    }

}
