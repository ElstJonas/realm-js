import { AnonymousCredential } from "./AnonymousCredential";
import { UsernamePasswordCredential } from "./UsernamePasswordCredential";

export function anonymous() {
    return new AnonymousCredential();
}

export function usernamePassword(username: string, password: string) {
    return new UsernamePasswordCredential(username, password);
}

export { AnonymousCredential, UsernamePasswordCredential };
