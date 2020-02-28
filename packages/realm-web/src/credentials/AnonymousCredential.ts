// See https://github.com/mongodb/stitch-js-sdk/blob/master/packages/core/sdk/src/auth/providers/anonymous/AnonymousAuthProvider.ts

/**
 * The AnonymousCredential is a [[StitchCredential]] that logs in
 * using the [Anonymous Authentication Provider](https://docs.mongodb.com/stitch/authentication/anonymous/).
 */
export class AnonymousCredential implements Realm.Credential {
    public readonly providerName = "anon-user";
    public readonly providerType = "anon-user";
    public readonly material = {};
}
