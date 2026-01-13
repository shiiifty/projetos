package hva.app.employee;

import pt.tecnico.uilib.forms.Form;

import hva.Hotel;
import hva.app.exceptions.UnknownEmployeeKeyException;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;
//FIXME import other classes if needed

class DoAddResponsibility extends Command<Hotel> {

    DoAddResponsibility(Hotel receiver) {
        super(Label.ADD_RESPONSABILITY, receiver);
        
    }

    @Override
    protected void execute() throws CommandException {
        Form request = new Form();
        try {
            request.addStringField("employeeId", Prompt.employeeKey());
            request.addStringField("responsibilityId", Prompt.responsibilityKey());
            request.parse();
            _receiver.addResponsibility(request.stringField("employeeId"), request.stringField("responsibilityId"));
        } catch (hva.exceptions.UnknownEmployeeKeyException e) {throw new UnknownEmployeeKeyException(request.stringField("employeeId"));}
    }
}