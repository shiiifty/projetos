package hva.app.habitat;

import pt.tecnico.uilib.forms.Form;

import hva.Hotel;
import hva.trees.TreeType;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;

class DoShowNoTreesHabitats extends Command<Hotel> {

    DoShowNoTreesHabitats(Hotel receiver) {
        super(Label.NO_TREES_HABITAT, receiver);
    }

    @Override
    protected void execute() throws CommandException {
        String[] outputs = _receiver.noTreesHabitat();
        for (String output: outputs) _display.popup(output);
    }
}