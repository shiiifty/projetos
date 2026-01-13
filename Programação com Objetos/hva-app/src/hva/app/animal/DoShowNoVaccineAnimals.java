package hva.app.animal;

import pt.tecnico.uilib.forms.Form;

import hva.Hotel;

import pt.tecnico.uilib.forms.Form;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;



class DoShowNoVaccineAnimals extends Command<Hotel> {

    DoShowNoVaccineAnimals(Hotel receiver) {
        super(Label.NO_VACCINE_ANIMALS, receiver);
    }

    @Override
    protected final void execute() throws CommandException {
        String[] outputs = _receiver.noVaccineAnimals();
        for (String output: outputs) _display.popup(output);
    }
}