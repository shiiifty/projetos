package hva.exceptions;

import java.io.Serial;

public class DuplicateTreeKeyException extends Exception{
    @Serial
    private static final long serialVersionUID = 202407081733L;

    private final String key;

    public DuplicateTreeKeyException(String key) {
        this.key = key;
    }

    public String getKey() {
        return key;
    }
}