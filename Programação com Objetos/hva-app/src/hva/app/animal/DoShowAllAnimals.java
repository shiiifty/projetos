package hva.app.animal;

import hva.Hotel;
import pt.tecnico.uilib.menus.Command;


class DoShowAllAnimals extends Command<Hotel> {
    DoShowAllAnimals(Hotel receiver) {
        super(Label.SHOW_ALL_ANIMALS, receiver);
    }

    @Override
    protected final void execute() {
        String[] outputs = _receiver.getAllAnimals();
        for (String output: outputs) _display.popup(output);
    }
}

