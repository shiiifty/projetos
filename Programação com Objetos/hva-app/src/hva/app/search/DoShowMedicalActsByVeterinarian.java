package hva.app.search;

import pt.tecnico.uilib.forms.Form;

import hva.Hotel;
import hva.app.employee.Prompt;
import hva.app.exceptions.UnknownVeterinarianKeyException;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;
//FIXME import other classes if needed

class DoShowMedicalActsByVeterinarian extends Command<Hotel> {

    DoShowMedicalActsByVeterinarian(Hotel receiver) {
        super(Label.MEDICAL_ACTS_BY_VET, receiver);
    }

    @Override
    protected void execute() throws CommandException {
        Form request = new Form();
        try {
            request.addStringField("vetId", Prompt.employeeKey());
            request.parse();
            String[] outputs = _receiver.getVeterinarianMedicalActs(request.stringField("vetId"));
            for (String output: outputs) _display.popup(output);
        } catch (hva.exceptions.UnknownVeterinarianKeyException e) {throw new UnknownVeterinarianKeyException(request.stringField("vetId"));}
    }

}
