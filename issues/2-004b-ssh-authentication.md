# 2-004b: SSH Authentication

## Parent Issue
2-004: SSH Server Integration

## Current Behavior
No SSH authentication exists.

## Intended Behavior
SSH authentication system that:
- Supports password authentication
- Supports public key authentication
- Validates credentials against game server
- Creates session on successful auth
- Rejects invalid credentials gracefully

## Suggested Implementation Steps

1. Create `src/server/04-ssh.h` with auth types:
   ```c
   // {{{ auth types
   typedef enum {
       SSH_AUTH_PASSWORD,
       SSH_AUTH_PUBKEY,
       SSH_AUTH_NONE
   } SSHAuthMethod;

   typedef struct {
       char username[64];
       SSHAuthMethod method;
       bool authenticated;
   } SSHAuthState;
   // }}}
   ```

2. Implement password authentication callback:
   ```c
   // {{{ password auth callback
   static int auth_password_callback(ssh_session session,
                                      const char *user,
                                      const char *password,
                                      void *userdata) {
       SSHConnection* conn = (SSHConnection*)userdata;

       // For now, accept any non-empty password
       // In production, validate against user database
       if (strlen(password) > 0) {
           strncpy(conn->auth.username, user, sizeof(conn->auth.username) - 1);
           conn->auth.authenticated = true;
           return SSH_AUTH_SUCCESS;
       }

       return SSH_AUTH_DENIED;
   }
   // }}}
   ```

3. Implement public key authentication callback:
   ```c
   // {{{ pubkey auth callback
   static int auth_pubkey_callback(ssh_session session,
                                    const char *user,
                                    ssh_key pubkey,
                                    char signature_state,
                                    void *userdata) {
       SSHConnection* conn = (SSHConnection*)userdata;

       if (signature_state == SSH_PUBLICKEY_STATE_NONE) {
           // Client is probing, allow to continue
           return SSH_AUTH_SUCCESS;
       }

       if (signature_state == SSH_PUBLICKEY_STATE_VALID) {
           strncpy(conn->auth.username, user, sizeof(conn->auth.username) - 1);
           conn->auth.authenticated = true;
           return SSH_AUTH_SUCCESS;
       }

       return SSH_AUTH_DENIED;
   }
   // }}}
   ```

4. Set up server callbacks:
   ```c
   // {{{ setup auth callbacks
   void ssh_setup_auth_callbacks(ssh_session session, SSHConnection* conn) {
       struct ssh_server_callbacks_struct cb = {
           .auth_password_function = auth_password_callback,
           .auth_pubkey_function = auth_pubkey_callback,
           .userdata = conn
       };
       ssh_callbacks_init(&cb);
       ssh_set_server_callbacks(session, &cb);
   }
   // }}}
   ```

5. Generate server host key on first run:
   ```c
   // {{{ generate host key
   void ssh_ensure_host_key(const char* key_path) {
       if (access(key_path, F_OK) != 0) {
           // Generate new RSA key
           ssh_key key;
           ssh_pki_generate(SSH_KEYTYPE_RSA, 2048, &key);
           ssh_pki_export_privkey_file(key, NULL, NULL, NULL, key_path);
           ssh_key_free(key);
       }
   }
   // }}}
   ```

6. Add auth timeout handling

7. Log authentication attempts

8. Write tests for auth scenarios

## Related Documents
- 2-004a-libssh-build-integration.md
- 2-001-configuration-system.md

## Dependencies
- 2-004a: libssh Build Integration

## Security Considerations
- Rate limit failed auth attempts
- Log all auth attempts for audit
- Support key-based auth for security
- Never store plaintext passwords

## Acceptance Criteria
- [ ] Password auth works
- [ ] Public key auth works
- [ ] Invalid credentials rejected
- [ ] Host key generated if missing
- [ ] Auth attempts logged
- [ ] Timeout on slow auth
