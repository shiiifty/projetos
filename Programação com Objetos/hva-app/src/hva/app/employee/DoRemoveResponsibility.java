package hva.app.employee;

import pt.tecnico.uilib.forms.Form;

import hva.Hotel;
import hva.app.exceptions.UnknownEmployeeKeyException;
import hva.app.exceptions.NoResponsibilityException;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;


class DoRemoveResponsibility extends Command<Hotel> {

    DoRemoveResponsibility(Hotel receiver) {
        super(Label.REMOVE_RESPONSABILITY, receiver);
    }

    @Override
    protected void execute() throws CommandException {
        Form request = new Form();
        try {
            request.addStringField("employeeId", Prompt.employeeKey());
            request.addStringField("responsibilityId", Prompt.responsibilityKey());
            request.parse();
            _receiver.removeResponsibility(request.stringField("employeeId"), request.stringField("responsibilityId"));
        } catch (hva.exceptions.UnknownEmployeeKeyException e) {throw new UnknownEmployeeKeyException(request.stringField("employeeId"));}
        catch (hva.exceptions.NoResponsibilityException e) {throw new NoResponsibilityException(request.stringField("employeeId"), request.stringField("responsibilityId"));}
    }
}