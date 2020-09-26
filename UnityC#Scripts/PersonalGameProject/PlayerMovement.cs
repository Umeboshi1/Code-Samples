using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using UnityEditorInternal;
using UnityEngine;

public class PlayerMovement : MonoBehaviour
{
    Rigidbody rb;
    CapsuleCollider col;

    public float moveSpeed;
    public float jumpSpeed;
    public float minJumpSpeed;

    public float gravity;
    float fallSpeed;

    //capsulecast
    Vector3 point1;
    Vector3 point2;
    float radius;

    public bool collide;

    public bool grounded;

    public float slopeForce;
    public float slopeRayLength;

    //Input buffers
    float aBuffer;
    float groundedBuf;

    public float inputBufferTime;
    public float groundedBufferTime;

    public float acceleration;
    public float deceleration;

    bool jumped;

    public bool isFacingRight;

    public float reboundSpeedUp;
    public float reboundSpeedHorizontal;


    // Start is called before the first frame update
    void Start()
    {
        rb = GetComponent<Rigidbody>();
        rb.useGravity = false;
        col = GetComponent<CapsuleCollider>();

        radius = col.radius * .90f;
        radius *= transform.localScale.x;
        float halfHeight = (col.height / 2 - col.radius);
        halfHeight *= transform.localScale.y;
        point1 = col.center + Vector3.up * halfHeight;
        point2 = col.center - Vector3.up * halfHeight;
        jumped = false;
        isFacingRight = true;
    }

    //Check if the character is standing on a slope
    private bool onSlope()
    {
        RaycastHit hit;

        if(Physics.Raycast(transform.position, Vector3.down, out hit, col.height/2 * slopeRayLength))
        {
            if(hit.normal != Vector3.up)
            {
                return true;
            }
        }
        return false;
    }

    //Get the normal direction of the slope
    private Vector3 slopeTangent()
    {
        RaycastHit hit;

        if (Physics.Raycast(transform.position, Vector3.down, out hit, col.height / 2 * slopeRayLength))
        {
            return hit.normal;
        }
        return Vector3.zero;
    }

    // Update is called once per frame
    void Update()
    {
        //decrement all buffer variables
        aBuffer -= Time.deltaTime;
        groundedBuf -= Time.deltaTime;


        float horizontal = Input.GetAxis("LeftJoystickHorizontal");
        if(horizontal > 0.2f)
        {
            isFacingRight = true;
        } else if(horizontal < -0.2f)
        {
            isFacingRight = false;
        }

        //check for collisions
        RaycastHit[] hitsHorizontal = Physics.CapsuleCastAll(transform.position + point1, transform.position + point2, radius, horizontal * Vector3.right, (transform.localScale.x * .2f));
        RaycastHit[] hitsVertical = Physics.CapsuleCastAll(transform.position + point1, transform.position + point2, radius * .7f, Vector3.down, (transform.localScale.x * .3f));

        if(!(groundedBuf > 0))
        {
            grounded = false;
        }

        foreach(RaycastHit hit in hitsVertical)
        {
            if (hit.collider.tag == "Wall")
            {
                groundedBuf = groundedBufferTime;
                jumped = false;
                grounded = true;
            }
        }

        bool isWall = false;

        foreach(RaycastHit hit in hitsHorizontal)
        {
            if(hit.collider.tag == "Wall")
            {
                isWall = true;
            }
        }

        /*if(grounded && horizontal != 0 && (onSlope()))
        {
            rb.velocity = new Vector3(rb.velocity.x, rb.velocity.y + (gravity * Time.deltaTime) - slopeForce);
        }*/

        if (!isWall)
        {
            rb.velocity = Vector3.Lerp(rb.velocity, new Vector3(horizontal * moveSpeed, rb.velocity.y, 0), acceleration);
        } else
        {
            rb.velocity = Vector3.Lerp(rb.velocity, new Vector3(0, rb.velocity.y, 0), deceleration);
        }

        if (!grounded)
        {
            rb.velocity = new Vector3(rb.velocity.x, rb.velocity.y + gravity * Time.deltaTime);
        } else
        {
            if (onSlope())
            {
                Vector3 tangent = slopeTangent();
                //Debug.Log(tangent.x + " " + tangent.y);
                rb.velocity = new Vector3(rb.velocity.x - tangent.x, rb.velocity.y - tangent.y, 0);
            } else
            {
                rb.velocity = new Vector3(rb.velocity.x, 0.0f, 0.0f);
            }
        }

        collide = isWall;

        if (Input.GetButtonDown("AButton"))
        {
            aBuffer = inputBufferTime;
        }

        if ((aBuffer > 0) && grounded) //jump
        {
            rb.velocity = new Vector3(rb.velocity.x, jumpSpeed, 0);
            jumped = true;
        }

        if(Input.GetButtonUp("AButton") && jumped)
        {
            if(rb.velocity.y > minJumpSpeed)
            {
                rb.velocity = new Vector3(rb.velocity.x, minJumpSpeed, 0.0f);
            }
        }

    }

    public void OnTriggerEnter(Collider other)
    {
        if(other.tag == "Hazard")
        {
            //Debug.Log(other.name + " hazard touched");
        }
    }

    public void OnTriggerStay(Collider other)
    {
        if(other.tag == "Hazard")
        {
            //Debug.Log(other.name + " hazard stay");
        }
    }

    public void OnTriggerExit(Collider other)
    {
        if(other.tag == "Hazard")
        {
            //Debug.Log(other.name + " hazard exit");
        }
    }

    public void rebound(GameObject hitDir)
    {
        switch (hitDir.name)
        {
            case "DownHit":
                    rb.velocity = new Vector3(rb.velocity.x, reboundSpeedUp, 0.0f);
                break;
            case "LeftHit":
                rb.velocity = new Vector3(rb.velocity.x + reboundSpeedHorizontal, rb.velocity.y, 0.0f);
                break;
            case "RightHit":
                rb.velocity = new Vector3(rb.velocity.x - reboundSpeedHorizontal, rb.velocity.y, 0.0f);
                break;
        }
    }
}
