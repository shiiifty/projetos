package hva.app.vaccine;

import hva.Hotel;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;
//FIXME import other classes if needed

class DoShowAllVaccines extends Command<Hotel> {

    DoShowAllVaccines(Hotel receiver) {
        super(Label.SHOW_ALL_VACCINES, receiver);
    }

    @Override
    protected final void execute() {
        String[] outputs = _receiver.getAllVaccines();
        for (String output: outputs) _display.popup(output);
    }
}