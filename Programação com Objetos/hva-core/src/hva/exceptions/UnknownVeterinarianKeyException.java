package hva.exceptions;

import java.io.Serial;

public class UnknownVeterinarianKeyException extends Exception {
    @Serial
    private static final long serialVersionUID = 202407081733L;

    private final String key;

    public UnknownVeterinarianKeyException(String key) {
        this.key = key;
    }

    public String getKey() {
        return key;
    }
}